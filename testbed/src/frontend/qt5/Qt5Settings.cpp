#include <qt5/Qt5Settings.h>
#include <qt5/Qt5Layout.h>
#include <qt5/Qt5MainWindow.h>

#include <QSettings>
#include <QFile>
#include <QXmlStreamReader>
#include <QCoreApplication>

#if NcFeature(NcPlatformWindows)
#include <QPlastiqueStyle>
#endif

namespace prodbg
{

Qt5Settings::Qt5Settings(Qt5MainWindow* mainWindow)
: m_mainWindow(mainWindow)
, m_settingCount(0)
{
	resetSettings();

	m_settingIdMap.insert(QString("Reset"), Qt5SettingId_Reset);

	// General settings
	m_settingIdMap.insert(QString("OpaqueSplitter"),     Qt5SettingId_OpaqueSplitter);
	m_settingIdMap.insert(QString("OpaqueDockSplitter"), Qt5SettingId_OpaqueDockSplitter);

	// Windows settings
	m_settingIdMap.insert(QString("QtStyleWin32"), Qt5SettingId_QtStyleWin32);

	m_argumentTypeMap.insert(QString("reset"),  Qt5SettingArgumentType_Reset);
	m_argumentTypeMap.insert(QString("int8"),   Qt5SettingArgumentType_Int8);
	m_argumentTypeMap.insert(QString("int16"),  Qt5SettingArgumentType_Int16);
	m_argumentTypeMap.insert(QString("int32"),  Qt5SettingArgumentType_Int32);
	m_argumentTypeMap.insert(QString("int64"),  Qt5SettingArgumentType_Int64);
	m_argumentTypeMap.insert(QString("uint8"),  Qt5SettingArgumentType_UInt8);
	m_argumentTypeMap.insert(QString("uint16"), Qt5SettingArgumentType_UInt16);
	m_argumentTypeMap.insert(QString("uint32"), Qt5SettingArgumentType_UInt32);
	m_argumentTypeMap.insert(QString("uint64"), Qt5SettingArgumentType_UInt16);
	m_argumentTypeMap.insert(QString("bool"),   Qt5SettingArgumentType_Bool);
	m_argumentTypeMap.insert(QString("string"), Qt5SettingArgumentType_String);
	m_argumentTypeMap.insert(QString("blob"),   Qt5SettingArgumentType_Blob);
}

Qt5Settings::~Qt5Settings()
{
}

void Qt5Settings::setSettingCount(int32 count)
{
	m_settingCount = count;
}

int32_t Qt5Settings::getSettingCount() const
{
	return m_settingCount;
}

void Qt5Settings::addSetting(Qt5Setting* setting)
{
	(void)setting;
}

Qt5Setting* Qt5Settings::getSetting(Qt5SettingId id)
{
	for (int32 index = 0; index < m_settingCount; ++index)
	{
		if (m_settings[index].id == id)
		{
			return &m_settings[index];
		}
	}

	return nullptr;
}

void Qt5Settings::resetSettings()
{
	for (int32 index = 0; index < QT5_MAX_SETTINGS; ++index)
	{
		m_settings[index].id = Qt5SettingId_Reset;
        resetArguments(&m_settings[index], true);
	}
}

void Qt5Settings::saveSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
	
	char8 idString[256];

	settings.setValue("Settings/Count", static_cast<int32>(m_settingCount));

	for(int32 index = 0; index < m_settingCount; ++index)
	{
		sprintf(idString, "Setting%i/Id", index);
		settings.setValue(idString, static_cast<int32>(m_settings[index].id));

        sprintf(idString, "Setting%i/ArgumentCount", index);
		settings.setValue(idString, static_cast<uint32>(m_settings[index].argumentCount));

        saveArguments(&m_settings[index], index);
	}
}

void Qt5Settings::loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
	
	char8 idString[256];

	if (!settings.contains("Settings/SettingCount"))
	{
		defaultSettings();
		m_mainWindow->triggerSignalSettings();
		return;
	}

	m_settingCount = settings.value("Settings/SettingCount").toInt();

	for (int32 index = 0; index < m_settingCount; ++index)
	{
		sprintf(idString, "Setting%i/Id", index);
		m_settings[index].id = static_cast<Qt5SettingId>(settings.value(idString).toInt());

		sprintf(idString, "Setting%i/ArgumentCount", index);
		m_settings[index].argumentCount = static_cast<uint32>(settings.value(idString).toUInt());

        loadArguments(&m_settings[index], index);
	}

	m_mainWindow->triggerSignalSettings();
}

void Qt5Settings::copySettings(Qt5Settings* dst)
{
	for (int32 index = 0; index < m_settingCount; ++index)
	{
		dst->m_settings[index].id = m_settings[index].id;
        copyArguments(&dst->m_settings[index], &m_settings[index]);
        dst->m_settings[index].argumentCount = m_settings[index].argumentCount;
	}
}

void Qt5Settings::defaultSettings()
{
	QFile xmlFile(":/DefaultSettings.xml");
	if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
         return;

	QXmlStreamReader xmlReader(&xmlFile);
    while (!xmlReader.atEnd())
	{
		switch (xmlReader.readNext())
		{
			case QXmlStreamReader::StartElement:
			{
				if (!xmlReader.name().compare(QString("Setting"), Qt::CaseInsensitive))
				{
					Qt5SettingId id = Qt5SettingId_Reset;
					Qt5SettingArgumentType type = Qt5SettingArgumentType_Reset;
					
					QString valueString;

					QXmlStreamAttributes itemAttributes = xmlReader.attributes();
					
					int32 attributeCount = 0;

					while (!itemAttributes.empty() && attributeCount < itemAttributes.count())
					{
						QXmlStreamAttribute itemAttribute = itemAttributes.at(attributeCount);

						if (!itemAttribute.name().compare(QString("Name"), Qt::CaseInsensitive))
						{
							id = m_settingIdMap.value(itemAttribute.value().toString().toUpper());
						}
						else if (!itemAttribute.name().compare(QString("Type"), Qt::CaseInsensitive))
						{
							type = m_argumentTypeMap.value(itemAttribute.value().toString().toUpper());
						}
						else if (!itemAttribute.name().compare(QString("Value"), Qt::CaseInsensitive))
						{
							valueString = itemAttribute.value().toString();
						}

						++attributeCount;
					}

					if (id > 0)
					{
						Qt5Setting setting;
						setting.id = id;
						g_settings->resetArguments(&setting, true);

						switch (type)
						{
							case Qt5SettingArgumentType_Int8:
							{
								int8 value = static_cast<int8>(valueString.toShort());
								addArgument(&setting, type, &value, sizeof(int8));
								break;
							}

							case Qt5SettingArgumentType_Int16:
							{
								int16 value = static_cast<int16>(valueString.toShort());
								addArgument(&setting, type, &value, sizeof(int16));
								break;
							}

							case Qt5SettingArgumentType_Int32:
							{
								int32 value = static_cast<int32>(valueString.toInt());
								addArgument(&setting, type, &value, sizeof(int32));
								break;
							}

							case Qt5SettingArgumentType_Int64:
							{
								int64 value = static_cast<int64>(valueString.toLongLong());
								addArgument(&setting, type, &value, sizeof(int64));
								break;
							}

							case Qt5SettingArgumentType_UInt8:
							{
								uint8 value = static_cast<uint8>(valueString.toUShort());
								addArgument(&setting, type, &value, sizeof(uint8));
								break;
							}

							case Qt5SettingArgumentType_UInt16:
							{
								uint16 value = static_cast<uint16>(valueString.toUShort());
								addArgument(&setting, type, &value, sizeof(uint16));
								break;
							}

							case Qt5SettingArgumentType_UInt32:
							{
								uint32 value = static_cast<uint32>(valueString.toUInt());
								addArgument(&setting, type, &value, sizeof(uint32));
								break;
							}

							case Qt5SettingArgumentType_UInt64:
							{
								uint64 value = static_cast<uint64>(valueString.toULongLong());
								addArgument(&setting, type, &value, sizeof(uint64));
								break;
							}

							case Qt5SettingArgumentType_Bool:
							{
								bool value = static_cast<int8>(valueString.toShort());
								addArgument(&setting, type, &value, sizeof(bool));
								break;
							}

							case Qt5SettingArgumentType_String:
							{
								QByteArray value = valueString.toLatin1();
								addArgument(&setting, type, value.data(), sizeof(value.length()));
								break;
							}

							case Qt5SettingArgumentType_Blob:
							{
								QByteArray value = valueString.toLatin1();
								addArgument(&setting, type, value.data(), sizeof(value.length()));
								break;
							}

							default:
							{
								addArgument(&setting, Qt5SettingArgumentType_Reset, nullptr, 0);
								break;
							}

						}
						
						addSetting(&setting);
					}
				}

				break;
			}

			default:
				break;
		}
	}
}

void Qt5Settings::addArgument(Qt5Setting* setting, Qt5SettingArgumentType type, void* dataPointer, uint64 dataSize)
{
	(void)setting;
	(void)type;
	(void)dataPointer;
	(void)dataSize;
}

Qt5SettingArgument* Qt5Settings::getArgument(Qt5Setting* setting, uint32 id)
{
	return &setting->arguments[id];
}

void Qt5Settings::copyArgument(Qt5SettingArgument* dst, const Qt5SettingArgument& src)
{
	(void)dst;
	(void)src;
}

void Qt5Settings::copyArguments(Qt5Setting* dst, Qt5Setting* src)
{
	for (uint32 argumentId = 0; argumentId < QT5_MAX_SETTING_ARGUMENTS; ++argumentId)
    {
        copyArgument(&dst->arguments[argumentId], src->arguments[argumentId]);
    }
}

void Qt5Settings::resetArgument(Qt5SettingArgument* dst, bool initialize)
{
	switch (dst->type)
    {
        case Qt5SettingArgumentType_String:
        case Qt5SettingArgumentType_Blob:
        {
            if (dst->dataPointer != nullptr && !initialize)
            {
                free(dst->dataPointer);
            }

            break;
        }

        case Qt5SettingArgumentType_Int8:
        case Qt5SettingArgumentType_Int16:
        case Qt5SettingArgumentType_Int32:
        case Qt5SettingArgumentType_UInt8:
        case Qt5SettingArgumentType_UInt16:
        case Qt5SettingArgumentType_UInt32:
        case Qt5SettingArgumentType_Bool:
        case Qt5SettingArgumentType_Int64:
        case Qt5SettingArgumentType_UInt64:
        default:
            break;
    }

    dst->type = Qt5SettingArgumentType_Reset;

    dst->dataPointer = nullptr;
    dst->dataSize = 0;
}

void Qt5Settings::resetArguments(Qt5Setting* setting, bool initialize)
{
	for (uint32 argumentId = 0; argumentId < QT5_MAX_SETTING_ARGUMENTS; ++argumentId)
    {
        resetArgument(&setting->arguments[argumentId], initialize);
    }

    setting->argumentCount = 0;
}

void Qt5Settings::saveArgument(Qt5Setting* setting, const uint32 settingIndex, const uint32 argumentIndex)
{
	(void)setting;
	(void)settingIndex;
	(void)argumentIndex;
}

void Qt5Settings::saveArguments(Qt5Setting* setting, const uint32 settingIndex)
{
	for (uint32 argumentId = 0; argumentId < setting->argumentCount; ++argumentId)
    {
        saveArgument(setting, settingIndex, argumentId);
    }
}

void Qt5Settings::loadArgument(Qt5Setting* setting, const uint32 settingIndex, const uint32 argumentIndex)
{
	(void)setting;
	(void)settingIndex;
	(void)argumentIndex;
}

void Qt5Settings::loadArguments(Qt5Setting* setting, const uint32 settingIndex)
{
	for (uint32 argumentId = 0; argumentId < setting->argumentCount; ++argumentId)
    {
        loadArgument(setting, settingIndex, argumentId);
    }
}

void Qt5Settings::saveLayout(Qt5Layout* layout)
{
#if NcFeature(Qt5SettingsDebugLayout)
	debugLayout(layout);
#endif

	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

	char8 idString[256];

	settings.remove("Layout");
	settings.setValue("Layout/EntryCount", layout->entryCount);

	for (int32 index = 0; index < layout->entryCount; ++index)
	{
		sprintf(idString, "Layout/Entry%i/EntryId", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].entryId));
		
		sprintf(idString, "Layout/Entry%i/ViewType", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].viewType));

		sprintf(idString, "Layout/Entry%i/ParentId", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].parentId));

		sprintf(idString, "Layout/Entry%i/PositionX", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].positionX));

		sprintf(idString, "Layout/Entry%i/PositionY", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].positionY));

		sprintf(idString, "Layout/Entry%i/SizeX", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].sizeX));

		sprintf(idString, "Layout/Entry%i/SizeY", index);
		settings.setValue(idString, static_cast<int32>(layout->entries[index].sizeY));

        if (layout->entries[index].extendedData.type == Qt5SettingArgumentType_String)
        {
            if (layout->entries[index].extendedData.dataPointer)
            {
                sprintf(idString, "Layout/Entry%i/ExtendedData", index);
                settings.setValue(idString, static_cast<char8*>(layout->entries[index].extendedData.dataPointer));
            }
        }

        if (layout->entries[index].viewType == Qt5ViewType_Main)
		{
			sprintf(idString, "Layout/Entry%i/IsMaximized", index);
			settings.setValue(idString, static_cast<bool>(layout->entries[index].isMaximized));

			sprintf(idString, "Layout/Entry%i/MainWindowState", index);
			settings.setValue(idString, static_cast<QByteArray&>(*layout->entries[index].mainWindowState));
		}
		else if (layout->entries[index].viewType == Qt5ViewType_Dynamic)
		{
			sprintf(idString, "Layout/Entry%i/Child1", index);
			settings.setValue(idString, static_cast<int32>(layout->entries[index].child1));

			sprintf(idString, "Layout/Entry%i/Child2", index);
			settings.setValue(idString, static_cast<int32>(layout->entries[index].child2));

			sprintf(idString, "Layout/Entry%i/IsFloating", index);
			settings.setValue(idString, static_cast<bool>(layout->entries[index].isFloating));

			sprintf(idString, "Layout/Entry%i/HasSplitter", index);
			settings.setValue(idString, static_cast<bool>(layout->entries[index].hasSplitter));

			sprintf(idString, "Layout/Entry%i/FillMainWindow", index);
			settings.setValue(idString, static_cast<bool>(layout->entries[index].fillMainWindow));

			sprintf(idString, "Layout/Entry%i/TopLevel", index);
			settings.setValue(idString, static_cast<bool>(layout->entries[index].topLevel));

			if (layout->entries[index].hasSplitter)
			{
				sprintf(idString, "Layout/Entry%i/SplitRegion1Size", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].splitRegion1Size));

				sprintf(idString, "Layout/Entry%i/SplitRegion2Size", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].splitRegion2Size));

				sprintf(idString, "Layout/Entry%i/SplitDirection", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].splitDirection));
			}

			if (!layout->entries[index].fillMainWindow)
			{
				sprintf(idString, "Layout/Entry%i/DockPositionX", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].dockPositionX));

				sprintf(idString, "Layout/Entry%i/DockPositionY", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].dockPositionY));

				sprintf(idString, "Layout/Entry%i/DockSizeX", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].dockSizeX));

				sprintf(idString, "Layout/Entry%i/DockSizeY", index);
				settings.setValue(idString, static_cast<int32>(layout->entries[index].dockSizeY));
			}
		}
	}
}

void Qt5Settings::loadLayout(Qt5Layout* layout)
{
	(void)layout;
}

#if NcFeature(Qt5SettingsDebugLayout)
void Qt5Settings::debugLayout(Qt5Layout* layout)
{
	for (int32 index = 0; index < layout->entryCount; ++index)
	{
		printf("View[%d]\n",     index);
		printf("\tId: %d\n",     layout->entries[index].entryId);
		printf("\tType: %d\n",   layout->entries[index].viewType);
		printf("\tParent: %d\n", layout->entries[index].parentId);
		printf("\tPosX: %d\n",   layout->entries[index].positionX);
		printf("\tPosY: %d\n",   layout->entries[index].positionY);

		if (layout->entries[index].viewType == Qt5ViewType_Dynamic)
		{
			printf("\tDynamic View Properties:\n");
			printf("\t\tChild1: %d\n",           layout->entries[index].child1);
			printf("\t\tChild2: %d\n",           layout->entries[index].child2);
			printf("\t\tIs Floating: %d\n",      layout->entries[index].isFloating);
			printf("\t\tHas Splitter: %d\n",     layout->entries[index].hasSplitter);
			printf("\t\tTop Level: %d\n",        layout->entries[index].topLevel);
			printf("\t\tFill Main Window: %d\n", layout->entries[index].fillMainWindow);
		}

		printf("\n");
	}
}
#endif

void Qt5Settings::resetEntry(Qt5LayoutEntry* entry)
{
	entry->entryId = 0;
	entry->positionX = 0;
	entry->positionY = 0;
	entry->sizeX = 0;
	entry->sizeY = 0;
	entry->parentId = 0;
	entry->viewType = Qt5ViewType_Init;

	// Main Window Parameters
	entry->isMaximized = false;
	entry->mainWindowState = nullptr;

	// Dynamic View Parameters
	entry->splitRegion1Size = 0;
	entry->splitRegion2Size = 0;
	entry->splitDirection = Qt::Horizontal;
	entry->child1 = 0;
	entry->child2 = 0;
	entry->dockPositionX = 0;
	entry->dockPositionY = 0;
	entry->dockSizeX = 0;
	entry->dockSizeY = 0;
	entry->isFloating = false;
	entry->hasSplitter = false;
	entry->fillMainWindow = false;
	entry->topLevel = false;
    entry->extendedData.dataPointer = nullptr;
    entry->extendedData.dataSize = 0;
    entry->extendedData.type = Qt5SettingArgumentType_Reset;
}

}