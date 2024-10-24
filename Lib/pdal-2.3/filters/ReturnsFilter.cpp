/******************************************************************************
 * Copyright (c) 2018, Bradley J Chambers (brad.chambers@gmail.com)
 ****************************************************************************/

#include "ReturnsFilter.h"

#include <pdal/util/ProgramArgs.h>

namespace pdal
{
	static StaticPluginInfo const s_info
	{
		"filters.returns", 
		"Split data by return order",
		"http://pdal.io/stages/filters.returns.html" 
	};

	CREATE_STATIC_STAGE(ReturnsFilter, s_info)

	std::string ReturnsFilter::getName() const
	{
		return s_info.name;
	}

	void ReturnsFilter::addArgs(ProgramArgs& args)
	{
		args.add("groups",
			"Comma-separated list of return number groupings ('first', "
			"'last', 'intermediate', or 'only')",
			m_returnsString, { "last" });
	}

	void ReturnsFilter::prepared(PointTableRef table)
	{
		const PointLayoutPtr layout(table.layout());
		if (!layout->hasDim(Dimension::Id::ReturnNumber) ||
			!layout->hasDim(Dimension::Id::NumberOfReturns))
		{
			log()->get(LogLevel::Warning)
				<< "Could not find ReturnNumber or "
				"NumberOfReturns. Proceeding with all returns.\n";
		}
	}

	PointViewSet ReturnsFilter::run(PointViewPtr inView)
	{
		PointViewSet viewSet;
		if (!inView->size())
			return viewSet;

		m_outputTypes = 0;
		for (auto& r : m_returnsString)
		{
			Utils::trim(r);
			if (r == "first")
				m_outputTypes |= returnFirst;
			else if (r == "intermediate")
				m_outputTypes |= returnIntermediate;
			else if (r == "last")
				m_outputTypes |= returnLast;
			else if (r == "only")
				m_outputTypes |= returnOnly;
			else
				throwError("Invalid output type: '" + r + "'.");
		}

		PointViewPtr firstView = inView->makeNew();
		PointViewPtr intermediateView = inView->makeNew();
		PointViewPtr lastView = inView->makeNew();
		PointViewPtr onlyView = inView->makeNew();

		for (PointId idx = 0; idx < inView->size(); idx++)
		{
			PointRef p = inView->point(idx);
			uint8_t rn = p.getFieldAs<uint8_t>(Dimension::Id::ReturnNumber);
			uint8_t nr = p.getFieldAs<uint8_t>(Dimension::Id::NumberOfReturns);
			if ((m_outputTypes & returnFirst) && (rn == 1) && (nr > 1))
				firstView->appendPoint(*inView.get(), idx);
			if ((m_outputTypes & returnIntermediate) && (rn > 1) && (rn < nr) &&
				(nr > 2))
				intermediateView->appendPoint(*inView.get(), idx);
			if ((m_outputTypes & returnLast) && (rn == nr) && (nr > 1))
				lastView->appendPoint(*inView.get(), idx);
			if ((m_outputTypes & returnOnly) && (nr == 1))
				onlyView->appendPoint(*inView.get(), idx);
		}

		if (m_outputTypes & returnFirst)
		{
			if (firstView->size())
				viewSet.insert(firstView);
			else
				log()->get(LogLevel::Warning)
				<< "Requested returns group 'first' is empty\n";
		}

		if (m_outputTypes & returnIntermediate)
		{
			if (intermediateView->size())
				viewSet.insert(intermediateView);
			else
				log()->get(LogLevel::Warning)
				<< "Requested returns group 'intermediate' is empty\n";
		}

		if (m_outputTypes & returnLast)
		{
			if (lastView->size())
				viewSet.insert(lastView);
			else
				log()->get(LogLevel::Warning)
				<< "Requested returns group 'last' is empty\n";
		}

		if (m_outputTypes & returnOnly)
		{
			if (onlyView->size())
				viewSet.insert(onlyView);
			else
				log()->get(LogLevel::Warning)
				<< "Requested returns group 'only' is empty\n";
		}

		return viewSet;
	}
}
