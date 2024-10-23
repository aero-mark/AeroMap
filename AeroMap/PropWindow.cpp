// PropWindow.cpp
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "AeroMap.h"			// application header
#include "PropWindow.h"

PropWindow::PropWindow(QWidget* parent)
	: QWidget(parent)
	, mp_ItemTool(nullptr)
{
	setupUi(this);
	setMinimumSize(235, 200);

	treeProp->setHeaderLabels(QStringList({ "Property", "Value" }));

	verify_connect(&GetApp()->m_Tool, SIGNAL(tool_change_sig()), this, SLOT(OnToolChange()));
	verify_connect(treeProp, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemClicked(QTreeWidgetItem*, int)));
	verify_connect(cmdApply, SIGNAL(clicked()), this, SLOT(OnApplyClicked()));
}

PropWindow::~PropWindow()
{
	delete mp_ItemTool;
}

void PropWindow::OnToolChange()
{
	LoadProperties();
}

void PropWindow::OnItemClicked(QTreeWidgetItem* item, int /* column */)
{
	QString str0 = item->text(0);
	QString str1 = item->text(1);

	txtPropName->setText(str0);
	txtPropValue->setText(str1);
}

void PropWindow::OnApplyClicked()
{
	if (treeProp->selectedItems().size() != 1)
		return;

	PropId propId = (PropId)(treeProp->selectedItems()[0]->data(0, Qt::UserRole).toInt());
	XString val = txtPropValue->text();

	switch (propId)
	{
	case PropId::Depth:
		GetApp()->m_Tool.SetMaxDepth(val.GetDouble());
		break;
	case PropId::Diameter:
		GetApp()->m_Tool.SetMaskSize(val.GetInt());
		break;
	case PropId::None:
		break;
	}
}

void PropWindow::LoadProperties()
{
	// Configure the property window for the 
	// selected tool.
	// 
	// Inputs:
	//		tool = selected tool

	// 9/7/19
	// This is still experimental - i definitely want a 
	// property sheet for each tool type, but am not at 
	// all clear on the implementation. Maybe i should have
	// a tool base class with different tool subclasses, maybe
	// something more hard-coded like this is correct, or 
	// maybe something completely different.

	switch (GetApp()->m_Tool.GetTool()) {
	case Tool::ToolType::None:				// no tool selected
		break;
	case Tool::ToolType::Router:			// cut path with router bit
		{
			char buf[32] = { 0 };

			delete mp_ItemTool;
			mp_ItemTool = new QTreeWidgetItem(QStringList("Router"));
			treeProp->addTopLevelItem(mp_ItemTool);

			sprintf(buf, "%d", GetApp()->m_Tool.GetMaskSize());

			QTreeWidgetItem* pItem0 = new QTreeWidgetItem();
			pItem0->setData(0, Qt::UserRole, (int)PropId::Diameter);
			pItem0->setText(0, "Diameter");
			pItem0->setText(1, buf);
			mp_ItemTool->addChild(pItem0);

			sprintf(buf, "%0.1f", GetApp()->m_Tool.GetMaxDepth());

			QTreeWidgetItem* pItem1 = new QTreeWidgetItem();
			pItem1->setData(0, Qt::UserRole, (int)PropId::Depth);
			pItem1->setText(0, "Depth");
			pItem1->setText(1, buf);
			mp_ItemTool->addChild(pItem1);
		}
		break;
	case Tool::ToolType::TexPaint:			// paint with texture
		//TODO:
		//maybe this is where i edit paintbrush
		break;
	case Tool::ToolType::WaterFill:			// flood-fill region with water-type vertices
	case Tool::ToolType::Lake:
	case Tool::ToolType::Ocean:
	case Tool::ToolType::Select:			// selecting drawing elements / geometry
	case Tool::ToolType::ViewZoom:			// zoom in/out
	case Tool::ToolType::ViewRotate:		// rotate view
	case Tool::ToolType::ViewPan:			// pan view
	case Tool::ToolType::Distance:			// measure linear distance
	case Tool::ToolType::Area:				// measure area
	case Tool::ToolType::Volume:
	case Tool::ToolType::Profile:
	case Tool::ToolType::Excavate:			// excavate rectangular region
	case Tool::ToolType::Light:				// move lights
		break;
	default:
		assert(false);
	}

	treeProp->expandAll();

	txtPropName->clear();
	txtPropValue->clear();
}
