// ProjectWindow.cpp
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "MainWindow.h"
#include "MetaDataDlg.h"
#include "TextFile.h"
#include "LightWindow.h"
#include "ProjectWindow.h"

#define DRONE_NODE_KEY	"ProjectWindow.DroneNodeState"
#define LIDAR_NODE_KEY	"ProjectWindow.LidarNodeState"

ProjectWindow::ProjectWindow(QWidget* parent)
	: QWidget(parent)
	, mp_Tree(nullptr)
	, mp_ItemRoot(nullptr)
	, mp_ItemDroneRoot(nullptr)
	, mp_ItemDroneInput(nullptr)
	, mp_ItemDroneOutput(nullptr)
	, mp_ItemLidarRoot(nullptr)
	, mp_actionActivate(nullptr)
	, mp_actionAddFile(nullptr)
	, mp_actionRemoveLidarFile(nullptr)
	, mp_actionOpenFolder(nullptr)
	, mp_actionProperties(nullptr)
	, mp_LightWindow(nullptr)
	, m_ItemChangeMask(false)
{
	setMinimumSize(300, 200);

	mp_gridLayout = new QGridLayout(this);

	mp_Tree = new QTreeWidget(this);
	mp_Tree->setColumnCount(1);
	mp_Tree->setHeaderHidden(true);
	mp_Tree->setExpandsOnDoubleClick(false);
	
	mp_gridLayout->addWidget(mp_Tree);

	CreateActions();

	// signals & slots
	verify_connect(mp_Tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemClicked(QTreeWidgetItem *, int)));
	verify_connect(mp_Tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemDoubleClicked(QTreeWidgetItem *, int)));
	verify_connect(&GetProject(), SIGNAL(projectChanged_sig()), this, SLOT(OnProjectChanged()));

	UpdateUI();
}

ProjectWindow::~ProjectWindow()
{
	delete mp_actionActivate;

	delete mp_actionAddFile;

	delete mp_actionRemoveLidarFile;

	delete mp_actionOpenFolder;
	delete mp_actionProperties;

	//delete mp_ItemRoot;
	//delete mp_ItemDroneRoot;
	//delete mp_ItemLidarRoot;

	//delete mp_LightWindow;
}

void ProjectWindow::closeEvent(QCloseEvent* event)
{
	// close event doesn't seem to get called for docked (child?) windows

	event->accept();
}

void ProjectWindow::UpdateUI()
{
	// Update the tree view.
	// 
	// Ideally would not have to rebuild
	//

	delete mp_ItemRoot;
	mp_ItemRoot = nullptr;

	mp_Tree->clear();
	mp_Tree->setRootIsDecorated(true);
	mp_Tree->setHeaderHidden(true);
	mp_Tree->setRootIsDecorated(true);

	// root node
	XString strProjectName = "Project";
	if (GetProject().GetName().IsEmpty() == false)
	{
		strProjectName += " - ";
		strProjectName += GetProject().GetName();
	}
	mp_ItemRoot = new QTreeWidgetItem(QStringList(strProjectName.c_str()), static_cast<int>(ItemType::ProjectRoot));
	mp_Tree->addTopLevelItem(mp_ItemRoot);

	mp_ItemDroneRoot = new QTreeWidgetItem(QStringList(tr("Photogrammetry")), static_cast<int>(ItemType::DroneRoot));
	mp_ItemLidarRoot = new QTreeWidgetItem(QStringList(tr("Lidar")), static_cast<int>(ItemType::LidarRoot));

	mp_ItemRoot->addChild(mp_ItemDroneRoot);
	mp_ItemRoot->addChild(mp_ItemLidarRoot);

	// photogrammetry items

	XString str = GetProject().GetDroneInputPath();
	if (QFile::exists(str.c_str()) == false)
		str.Insert(0, "(?) ");
	mp_ItemDroneInput = new QTreeWidgetItem(QStringList(str.c_str()), static_cast<int>(ItemType::DroneInput));
	mp_ItemDroneInput->setToolTip(0, str.c_str());
	mp_ItemDroneRoot->addChild(mp_ItemDroneInput);

	str = GetProject().GetDroneOutputPath();
	if (QFile::exists(str.c_str()) == false)
		str.Insert(0, "(?) ");
	mp_ItemDroneOutput = new QTreeWidgetItem(QStringList(str.c_str()), static_cast<int>(ItemType::DroneOutput));
	mp_ItemDroneOutput->setToolTip(0, str.c_str());
	mp_ItemDroneRoot->addChild(mp_ItemDroneOutput);

	// lidar items

	// for each las/laz file
	for (int i = 0; i < GetProject().GetLidarFileCount(); ++i)
	{
		XString str = GetProject().GetLidarFileName(i);
		if (GetProject().GetLidarExists(i) == false)
			str.Insert(0, "(?) ");

		QTreeWidgetItem* pItem = new QTreeWidgetItem(QStringList(str.c_str()), static_cast<int>(ItemType::LidarFile));
		pItem->setToolTip(0, str.c_str());
		mp_ItemLidarRoot->addChild(pItem);
	}

	QSettings settings(ORG_KEY, APP_KEY);
	if (settings.value(DRONE_NODE_KEY, false).toBool())
		mp_ItemDroneRoot->setExpanded(true);
	if (settings.value(LIDAR_NODE_KEY, false).toBool())
		mp_ItemLidarRoot->setExpanded(true);

	mp_ItemRoot->setExpanded(true);
}

void ProjectWindow::CreateActions()
{
	// workspace

	mp_actionActivate = new QAction(QIcon(""), tr("Activate View"), this);
	mp_actionActivate->setStatusTip(tr("Activate workspace view"));
	verify_connect(mp_actionActivate, SIGNAL(triggered()), this, SLOT(OnActivateView()));

	mp_actionAddFile = new QAction(QIcon(""), tr("Add File"), this);
	mp_actionAddFile->setStatusTip(tr("Add geospatial file to workspace"));
	verify_connect(mp_actionAddFile, SIGNAL(triggered()), this, SLOT(OnAddFile()));

	// lidar

	mp_actionRemoveLidarFile = new QAction(QIcon(""), tr("Remove Lidar File"), this);
	mp_actionRemoveLidarFile->setStatusTip(tr("Remove lidar file from project"));
	verify_connect(mp_actionRemoveLidarFile, SIGNAL(triggered()), this, SLOT(OnRemoveLidarFile()));

	// general

	mp_actionProperties = new QAction(QIcon(""), tr("Properties"), this);
	mp_actionProperties->setStatusTip(tr("Display item properties"));
	verify_connect(mp_actionProperties, SIGNAL(triggered()), this, SLOT(OnProperties()));

	mp_actionOpenFolder = new QAction(QIcon(""), tr("Open Folder"), this);
	mp_actionOpenFolder->setStatusTip(tr("Open containing folder"));
	verify_connect(mp_actionOpenFolder, SIGNAL(triggered()), this, SLOT(OnOpenFolder()));
}

void ProjectWindow::OnItemClicked(QTreeWidgetItem* pItem, int column)
{
	Q_UNUSED(pItem);
	Q_UNUSED(column);
}

void ProjectWindow::OnItemDoubleClicked(QTreeWidgetItem* pItem, int column)
{
	Q_UNUSED(column);

	// if its one of the project view types, activate it
	if (pItem == mp_ItemDroneRoot)
	{
		GetApp()->ActivateView(AeroMapApp::ViewType::Drone);
	}
	else if (pItem == mp_ItemDroneInput)
	{
		XString path = SelectFolder();
		if (path.GetLength() > 0)
		{
			GetProject().SetDroneInputPath(path);
			UpdateUI();
		}
	}
	else if (pItem == mp_ItemDroneOutput)
	{
		XString path = SelectFolder();
		if (path.GetLength() > 0)
		{
			GetProject().SetDroneOutputPath(path);
			UpdateUI();
		}
	}
	else
	{
		DisplayProperties(pItem);
	}
}

void ProjectWindow::OnActivateView()
{
	// Activate the workspace view.
	//

	if (mp_Tree->currentItem() != mp_ItemDroneRoot)
		return;

	GetApp()->ActivateView(AeroMapApp::ViewType::Drone);
}

void ProjectWindow::DisplayProperties(QTreeWidgetItem* pItem)
{
	ItemType itemType = static_cast<ItemType>(pItem->type());

	switch (itemType) {
	case ItemType::ProjectRoot:
		break;
	case ItemType::DroneRoot:
		break;
	case ItemType::DroneInput:
		break;
	default:
		break;
	}
}

void ProjectWindow::SaveState()
{
	// persist top-level node states

	QSettings settings(ORG_KEY, APP_KEY);
	settings.setValue(DRONE_NODE_KEY, mp_ItemDroneRoot->isExpanded());
	settings.setValue(LIDAR_NODE_KEY, mp_ItemLidarRoot->isExpanded());
}

void ProjectWindow::contextMenuEvent(QContextMenuEvent* event)
{
	// Context menus can be executed either asynchronously using the popup()
	// function or synchronously using the exec() function. In this example, 
	// we have chosen to show the menu using its exec() function. By passing 
	// the event's position as argument we ensure that the context menu appears 
	// at the expected position.

	int selectCount = mp_Tree->selectedItems().count();
	if (selectCount == 0)
		return;

	QMenu menu(this);

	ItemType itemType = static_cast<ItemType>(mp_Tree->selectedItems()[0]->type());

	// project folder can always be opened, but only 
	// certain sub-items have properties
	switch (itemType)
	{
	case ItemType::ProjectRoot:
		break;
	case ItemType::DroneRoot:
		menu.addAction(mp_actionActivate);
		menu.addAction(mp_actionAddFile);
		menu.addAction(mp_actionProperties);
		break;
	case ItemType::DroneInput:
		menu.addAction(mp_actionProperties);
		menu.addAction(mp_actionOpenFolder);
		break;
	case ItemType::DroneOutput:
		menu.addAction(mp_actionProperties);
		menu.addAction(mp_actionOpenFolder);
		break;
	case ItemType::LidarRoot:
		menu.addAction(mp_actionAddFile);
		break;
	case ItemType::LidarFile:
		menu.addAction(mp_actionRemoveLidarFile);
		menu.addAction(mp_actionOpenFolder);
		break;
	}

	menu.exec(event->globalPos());
}

void ProjectWindow::OnAddFile()
{
	// Handler for local context menu's
	// "Add File" action.
	//

	if (mp_Tree->selectedItems().count() == 0)
		return;

	XString defaultPath;
	ItemType itemType = static_cast<ItemType>(mp_Tree->selectedItems()[0]->type());

	// what "add file" means varies by project component
	switch (itemType) {
	case ItemType::LidarRoot:
		{
			XString filter = "Lidar Files (*.las *.laz)";

			XString fileName = QFileDialog::getOpenFileName(
				this,
				tr("Select File"),
				defaultPath.c_str(),
				filter.c_str());

			if (fileName.GetLength() > 0)
			{
				GetProject().AddLidarFile(fileName.c_str());
				UpdateUI();
			}
		}
		break;
	}
}

void ProjectWindow::OnRemoveLidarFile()
{
	// Handler for local context menu's
	// "Remove Lidar File" action.
	//

	for (int i = 0; i < mp_ItemLidarRoot->childCount(); ++i)
	{
		if (mp_ItemLidarRoot->child(i)->isSelected())
		{
			// remove file from project
			GetProject().RemoveLidarFile(i);
		}
	}
}

void ProjectWindow::OnLoadFile()
{
	// Handler for local context menu's
	// "Load File" action.
	//
	// This is for views that only support a single item
	// at a time, like terrain models.
	//

	if (mp_Tree->selectedItems().count() != 1)
		return;

	QTreeWidgetItem* pItem = mp_Tree->selectedItems()[0];
	if (pItem->parent() == nullptr)
		return;
	
	int index = pItem->parent()->indexOfChild(pItem);
	if (pItem->parent() == mp_ItemLidarRoot)
	{
		GetApp()->ActivateView(AeroMapApp::ViewType::Lidar, index);
	}
}

void ProjectWindow::OnProperties()
{
	// Handler for local context menu's
	// "Properties" action.

	if (mp_Tree->selectedItems().size() == 0)
		return;

	DisplayProperties(mp_Tree->selectedItems()[0]);
}

void ProjectWindow::OnOpenFolder()
{
	// Handler for local context menu's
	// "Open Folder" action.
	//

	if (mp_Tree->selectedItems().size() == 0)
		return;

	QTreeWidgetItem* pItem = mp_Tree->selectedItems()[0];
	if (pItem->parent() == nullptr)
		return;

	XString pathName;
	int layerIndex = pItem->parent()->indexOfChild(pItem);
	if (pItem->parent() == mp_ItemDroneRoot)
	{
	}
	else if (pItem->parent() == mp_ItemDroneRoot)
	{
	}
	else if (pItem->parent() == mp_ItemLidarRoot)
	{
		pathName = GetProject().GetLidarFileName(layerIndex);
	}

	if (pathName.IsEmpty() == false)
	{
		pathName = pathName.GetPathName();
		if (pathName.IsEmpty() == false)
			QDesktopServices::openUrl(QUrl::fromLocalFile(pathName.c_str()));
	}
}

XString ProjectWindow::SelectFolder()
{
	XString strFolder;

	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"),
		"/",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.length() > 0)
	{
		strFolder = dir;
		strFolder.Trim();
	}

	return strFolder;
}

void ProjectWindow::OnProjectChanged()
{
	UpdateUI();
}