#ifndef CHAMFERKERNEL_H
#define CHAMFERKERNEL_H

#include <pdal/Kernel.h>
#include <pdal/Stage.h>
#include <pdal/util/FileUtils.h>

namespace pdal
{
	class PointView;

	class ChamferKernel : public Kernel
	{
	public:
		std::string getName() const override;
		int execute() override;

	private:
		virtual void addSwitches(ProgramArgs& args);
		PointViewPtr loadSet(const std::string& filename, PointTableRef table);

		std::string m_sourceFile;
		std::string m_candidateFile;
	};
}

#endif // #ifndef CHAMFERKERNEL_H