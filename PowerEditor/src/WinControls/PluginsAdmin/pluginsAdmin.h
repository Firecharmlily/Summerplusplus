// This file is part of Notepad++ project
// Copyright (C)2017 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#include <cwctype>
#include <algorithm>
#include "StaticDialog.h"
#include "pluginsAdminRes.h"
#include "TabBar.h"
#include "ListView.h"
#include "tinyxml.h"

class PluginsManager;

struct Version
{
	unsigned long _major = 0;
	unsigned long _minor = 0;
	unsigned long _patch = 0;
	unsigned long _build = 0;

	Version() = default;
	Version(const generic_string& versionStr);

	void setVersionFrom(const generic_string& filePath);
	generic_string toString();
	bool isNumber(const generic_string& s) const {
		return !s.empty() && 
			find_if(s.begin(), s.end(), [](_TCHAR c) { return !_istdigit(c); }) == s.end();
	};

	int compareTo(const Version& v2c) const;

	bool operator < (const Version& v2c) const {
		return compareTo(v2c) == -1;
	};

	bool operator > (const Version& v2c) const {
		return compareTo(v2c) == 1;
	};

	bool operator == (const Version& v2c) const {
		return compareTo(v2c) == 0;
	};

	bool operator != (const Version& v2c) const {
		return compareTo(v2c) != 0;
	};
};

struct PluginUpdateInfo
{
	generic_string _fullFilePath; // only for the installed Plugin

	generic_string _folderName;   // plugin folder name - should be the same name with plugin and should be uniq among the plugins
	generic_string _displayName;  // plugin description name
	Version _version;
	generic_string _homepage;
	generic_string _sourceUrl;
	generic_string _description;
	generic_string _author;
	generic_string _id;           // Plugin package ID: SHA-256 hash
	generic_string _repository;
	bool _isVisible = true;       // if false then it should not be displayed 

	generic_string describe();
	PluginUpdateInfo() = default;
	PluginUpdateInfo(const generic_string& fullFilePath, const generic_string& fileName);
};

struct NppCurrentStatus
{
	bool _isAdminMode;              // can launch gitup en Admin mode directly

	bool _isInProgramFiles;         // true: install/update/remove on "Program files" (ADMIN MODE)
									// false: install/update/remove on NPP_INST or install on %APPDATA%, update/remove on %APPDATA% & NPP_INST (NORMAL MODE)
									
	bool _isAppDataPluginsAllowed;  // true: install on %APPDATA%, update / remove on %APPDATA% & "Program files" or NPP_INST

	generic_string _nppInstallPath;
	generic_string _appdataPath;

	// it should determinate :
	// 1. deployment location : %ProgramFile%   %appdata%   %other%
	// 2. gitup launch mode:    ADM             ADM         NOMAL
	bool shouldLaunchInAdmMode() { return _isInProgramFiles; };
};

enum COLUMN_TYPE { COLUMN_PLUGIN, COLUMN_VERSION };
enum SORT_TYPE { DISPLAY_NAME_ALPHABET_ENCREASE, DISPLAY_NAME_ALPHABET_DECREASE };


struct SortDisplayNameDecrease final
{
	bool operator() (PluginUpdateInfo* l, PluginUpdateInfo* r)
	{
		return (l->_displayName.compare(r->_displayName) <= 0);
	}
};

class PluginViewList
{
public:
	PluginViewList() = default;
	~PluginViewList() {
		_ui.destroy();
		for (auto i : _list)
		{
			delete i;
		}
	};

	void pushBack(PluginUpdateInfo* pi);
	HWND getViewHwnd() { return _ui.getHSelf(); };
	void displayView(bool doShow) const { _ui.display(doShow); };
	std::vector<size_t> getCheckedIndexes() const { return _ui.getCheckedIndexes(); };
	std::vector<PluginUpdateInfo*> fromUiIndexesToPluginInfos(const std::vector<size_t>& ) const;
	long getSelectedIndex() const { return _ui.getSelectedIndex(); };
	void setSelection(int index) const { _ui.setSelection(index); };
	void initView(HINSTANCE hInst, HWND parent) { _ui.init(hInst, parent); };
	void addColumn(const columnInfo & column2Add) { _ui.addColumn(column2Add); };
	void reSizeView(RECT & rc) { _ui.reSizeTo(rc); }
	void setViewStyleOption(int32_t extraStyle) { _ui.setStyleOption(extraStyle); };
	size_t nbItem() const { return _ui.nbItem(); };
	PluginUpdateInfo* getPluginInfoFromUiIndex(size_t index) const { return reinterpret_cast<PluginUpdateInfo*>(_ui.getLParamFromIndex(static_cast<int>(index))); };
	PluginUpdateInfo* findPluginInfoFromFolderName(const generic_string& folderName, int& index) const;
	bool removeFromListIndex(size_t index2remove);
	bool hideFromListIndex(size_t index2Hide);
	bool removeFromFolderName(const generic_string& folderName);
	bool removeFromUiIndex(size_t index2remove);
	bool hideFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2hide);
	bool restore(const generic_string& folderName);
	bool removeFromPluginInfoPtr(PluginUpdateInfo* pluginInfo2hide);
	void changeColumnName(COLUMN_TYPE index, const TCHAR *name2change);

private:
	std::vector<PluginUpdateInfo*> _list;
	ListView _ui;

	SORT_TYPE _sortType = DISPLAY_NAME_ALPHABET_ENCREASE;
};

enum LIST_TYPE { AVAILABLE_LIST, UPDATES_LIST, INSTALLED_LIST };


class PluginsAdminDlg final : public StaticDialog
{
public :
	PluginsAdminDlg();
	~PluginsAdminDlg() = default;

    void init(HINSTANCE hInst, HWND parent)	{
        Window::init(hInst, parent);
	};

	virtual void create(int dialogID, bool isRTL = false, bool msgDestParent = true);

    void doDialog(bool isRTL = false) {
    	if (!isCreated())
		{
			create(IDD_PLUGINSADMIN_DLG, isRTL);
		}

		if (!::IsWindowVisible(_hSelf))
		{

		}
	    display();
    };

	bool isValide();

	void switchDialog(int indexToSwitch);
	void setPluginsManager(PluginsManager *pluginsManager) { _pPluginsManager = pluginsManager; };

	bool updateListAndLoadFromJson();
	void setAdminMode(bool isAdm) { _nppCurrentStatus._isAdminMode = isAdm; };

	bool installPlugins();
	bool updatePlugins();
	bool removePlugins();

	void changeTabName(LIST_TYPE index, const TCHAR *name2change);
	void changeColumnName(COLUMN_TYPE index, const TCHAR *name2change);

protected:
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

private :
	generic_string _updaterDir;
	generic_string _updaterFullPath;
	generic_string _pluginListFullPath;

	TabBar _tab;

	PluginViewList _availableList; // A permanent list, once it's loaded (no removal - only hide or show) 
	PluginViewList _updateList;    // A dynamical list, items are removable
	PluginViewList _installedList; // A dynamical list, items are removable

	PluginsManager *_pPluginsManager = nullptr;
	NppCurrentStatus _nppCurrentStatus;

	void collectNppCurrentStatusInfos();
	bool searchInPlugins(bool isNextMode) const;
	const bool _inNames = true;
	const bool _inDescs = false;
	bool isFoundInAvailableListFromIndex(int index, const generic_string& str2search, bool inWhichPart) const;
	long searchFromCurrentSel(const generic_string& str2search, bool inWhichPart, bool isNextMode) const;
	long searchInNamesFromCurrentSel(const generic_string& str2search, bool isNextMode) const {
		return searchFromCurrentSel(str2search, _inNames, isNextMode);
	};

	long searchInDescsFromCurrentSel(const generic_string& str2search, bool isNextMode) const {
		return searchFromCurrentSel(str2search, _inDescs, isNextMode);
	};
	
	bool loadFromPluginInfos();
	bool checkUpdates();

	enum Operation {
		pa_install = 0,
		pa_update = 1,
		pa_remove = 2
	};
	bool exitToInstallRemovePlugins(Operation op, const std::vector<PluginUpdateInfo*>& puis);
};

