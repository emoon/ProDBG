#pragma once

#include "Qt5Layout.h"

#include <QMap>

#ifndef QT5_MAX_SETTINGS
#define QT5_MAX_SETTINGS 256
#endif

#ifndef QT5_MAX_SETTING_ARGUMENTS
#define QT5_MAX_SETTING_ARGUMENTS 128
#endif

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5Layout;
class Qt5LayoutEntry;
class Qt5MainWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum Qt5SettingId
{
	Qt5SettingId_Reset = 0,

	// 0x1000+ - General settings
	Qt5SettingId_OpaqueSplitter = 0x1000,
	Qt5SettingId_OpaqueDockSplitter = 0x1001,

	// 0x10000+ - Windows settings
	Qt5SettingId_QtStyleWin32 = 0x10000,

	// 0x20000+ - Mac settings
	
	// 0x30000+ - Unix\Linux settings
	
	// 0x50000+ - Plugin settings
};

enum Qt5SettingArgumentType
{
    Qt5SettingArgumentType_Reset  = 0x00,
    Qt5SettingArgumentType_Int8   = 0x01,
    Qt5SettingArgumentType_Int16  = 0x02,
    Qt5SettingArgumentType_Int32  = 0x03,
    Qt5SettingArgumentType_Int64  = 0x04,
    Qt5SettingArgumentType_UInt8  = 0x05,
    Qt5SettingArgumentType_UInt16 = 0x06,
    Qt5SettingArgumentType_UInt32 = 0x07,
    Qt5SettingArgumentType_UInt64 = 0x08,
    Qt5SettingArgumentType_Bool   = 0x09,
    Qt5SettingArgumentType_String = 0x10,
    Qt5SettingArgumentType_Blob   = 0x11,
};

struct Qt5SettingArgument
{
    Qt5SettingArgumentType type;
    void* dataPointer;
    uint64_t dataSize;
};

struct Qt5Setting
{
	Qt5SettingId id;
	Qt5SettingArgument arguments[QT5_MAX_SETTING_ARGUMENTS]; 
    uint32_t argumentCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5Settings
{
public:
	Qt5Settings(Qt5MainWindow* mainWindow);
	virtual ~Qt5Settings();

	void setSettingCount(int32_t count);
	int32_t getSettingCount() const;

	void addSetting(Qt5Setting* setting);
	Qt5Setting* getSetting(Qt5SettingId id);

	void resetSettings();
	void saveSettings();
	void loadSettings();
	void copySettings(Qt5Settings* dst);
	void defaultSettings();

	void addArgument(Qt5Setting* setting, Qt5SettingArgumentType type, void* dataPointer, uint64_t dataSize);
    Qt5SettingArgument* getArgument(Qt5Setting* setting, uint32_t id);
    
    void copyArgument(Qt5SettingArgument* dst, const Qt5SettingArgument& src);
    void copyArguments(Qt5Setting* dst, Qt5Setting* src);

    void resetArgument(Qt5SettingArgument* dst, bool initialize = false);
    void resetArguments(Qt5Setting* setting, bool initialize = false);

    void saveArgument(Qt5Setting* setting, const uint32_t settingIndex, const uint32_t argumentIndex);
    void saveArguments(Qt5Setting* setting, const uint32_t settingIndex);

    void loadArgument(Qt5Setting* setting, const uint32_t settingIndex, const uint32_t argumentIndex);
    void loadArguments(Qt5Setting* setting, const uint32_t settingIndex);

	void saveLayout(Qt5Layout* layout);
	void loadLayout(Qt5Layout* layout);

	void resetEntry(Qt5LayoutEntry* entry);

protected:
	Qt5MainWindow* m_mainWindow;

	QMap<QString, Qt5SettingArgumentType> m_argumentTypeMap;
	QMap<QString, Qt5SettingId> m_settingIdMap;

private:
	Qt5Setting m_settings[QT5_MAX_SETTINGS];
	int32_t m_settingCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}