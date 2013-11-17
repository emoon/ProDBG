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
	int32 index;

	for (index = 0; index < QT5_MAX_SETTINGS; ++index)
	{
		if (m_settings[index].id == setting->id)
		{
            resetArguments(&m_settings[index]);
			break;
		}
	}
	
	if (index >= QT5_MAX_SETTINGS || m_settingCount == 0)
	{
		for (index = 0; index < QT5_MAX_SETTINGS; ++index)
		{
			if (m_settings[index].id == Qt5SettingId_Reset)
			{
				++m_settingCount;
				break;
			}
		}
	}

	if (index >= QT5_MAX_SETTINGS)
		return;

	m_settings[index].id = setting->id;
    copyArguments(&m_settings[index], setting);
    m_settings[index].argumentCount = setting->argumentCount;
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
								addArgument(&setting, type, reinterpret_cast<int8*>(value), sizeof(int8));
								break;
							}

							case Qt5SettingArgumentType_Int16:
							{
								int16 value = static_cast<int16>(valueString.toShort());
								addArgument(&setting, type, reinterpret_cast<int16*>(value), sizeof(int16));
								break;
							}

							case Qt5SettingArgumentType_Int32:
							{
								int32 value = static_cast<int32>(valueString.toInt());
								addArgument(&setting, type, reinterpret_cast<int32*>(value), sizeof(int32));
								break;
							}

							case Qt5SettingArgumentType_Int64:
							{
								int64 value = static_cast<int64>(valueString.toLongLong());
								addArgument(&setting, type, reinterpret_cast<int64*>(value), sizeof(int64));
								break;
							}

							case Qt5SettingArgumentType_UInt8:
							{
								uint8 value = static_cast<uint8>(valueString.toUShort());
								addArgument(&setting, type, reinterpret_cast<uint8*>(value), sizeof(uint8));
								break;
							}

							case Qt5SettingArgumentType_UInt16:
							{
								uint16 value = static_cast<uint16>(valueString.toUShort());
								addArgument(&setting, type, reinterpret_cast<uint16*>(value), sizeof(uint16));
								break;
							}

							case Qt5SettingArgumentType_UInt32:
							{
								uint32 value = static_cast<uint32>(valueString.toUInt());
								addArgument(&setting, type, reinterpret_cast<uint32*>(value), sizeof(uint32));
								break;
							}

							case Qt5SettingArgumentType_UInt64:
							{
								uint64 value = static_cast<uint64>(valueString.toULongLong());
								addArgument(&setting, type, reinterpret_cast<uint64*>(value), sizeof(uint64));
								break;
							}

							case Qt5SettingArgumentType_Bool:
							{
								bool value = static_cast<int8>(valueString.toShort());
								addArgument(&setting, type, reinterpret_cast<bool*>(value), sizeof(bool));
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
	uint32 argumentId;

    for (argumentId = 0; argumentId < QT5_MAX_SETTING_ARGUMENTS; ++argumentId)
    {
        if (setting->arguments[argumentId].type == Qt5SettingArgumentType_Reset)
        {
            break;
        }
    }  

    if (argumentId >= QT5_MAX_SETTING_ARGUMENTS)
    {
        return;
    }

    setting->arguments[argumentId].type = type;
    setting->arguments[argumentId].dataSize = dataSize;

    switch (type)
    {
        case Qt5SettingArgumentType_Int8:
        case Qt5SettingArgumentType_Int16:
        case Qt5SettingArgumentType_Int32:
        case Qt5SettingArgumentType_UInt8:
        case Qt5SettingArgumentType_UInt16:
        case Qt5SettingArgumentType_UInt32:
        case Qt5SettingArgumentType_Bool:
        {
            setting->arguments[argumentId].dataPointer = dataPointer;
            break;
        }

        case Qt5SettingArgumentType_Int64:
        case Qt5SettingArgumentType_UInt64:
        {
            setting->arguments[argumentId].dataPointer = calloc(sizeof(int64), 1);
            memcpy(setting->arguments[argumentId].dataPointer, dataPointer, sizeof(int64));
            break;
        }

        case Qt5SettingArgumentType_String:
        {
            if (dataPointer)
            {
            	const uint32 stringLength = strlen(static_cast<char8*>(dataPointer));
                setting->arguments[argumentId].dataPointer = calloc(stringLength + 1, 1);
                memcpy(setting->arguments[argumentId].dataPointer, dataPointer, stringLength + 1);
            }
            else
            {
                setting->arguments[argumentId].dataPointer = nullptr;
            }

            break;
        }

        case Qt5SettingArgumentType_Blob:
        {
            if (dataPointer)
            {
                setting->arguments[argumentId].dataPointer = calloc(dataSize, 1);
                memcpy(setting->arguments[argumentId].dataPointer, dataPointer, dataSize);
            }
            else
            {
                setting->arguments[argumentId].dataPointer = nullptr;
            }

            break;
        }

        default:
            break;
    }

    ++setting->argumentCount;
}

Qt5SettingArgument* Qt5Settings::getArgument(Qt5Setting* setting, uint32 id)
{
	return &setting->arguments[id];
}

void Qt5Settings::copyArgument(Qt5SettingArgument* dst, const Qt5SettingArgument& src)
{
	dst->type = src.type;
    dst->dataSize = src.dataSize;
    
    switch (dst->type)
    {
        case Qt5SettingArgumentType_Int8:
        case Qt5SettingArgumentType_Int16:
        case Qt5SettingArgumentType_Int32:
        case Qt5SettingArgumentType_UInt8:
        case Qt5SettingArgumentType_UInt16:
        case Qt5SettingArgumentType_UInt32:
        case Qt5SettingArgumentType_Bool:
        {
            dst->dataPointer = src.dataPointer;
            break;
        }

        case Qt5SettingArgumentType_Int64:
        case Qt5SettingArgumentType_UInt64:
        {
            dst->dataPointer = calloc(sizeof(int64), 1);
            memcpy(dst->dataPointer, &src.dataPointer, sizeof(int64));
            break;
        }

        case Qt5SettingArgumentType_String:
        {
            if (src.dataPointer)
            {
            	const uint32 stringLength = strlen(static_cast<char8*>(src.dataPointer));
                dst->dataPointer = calloc(stringLength + 1, 1);
                memcpy(dst->dataPointer, src.dataPointer, stringLength + 1);
            }
            else
            {
                dst->dataPointer = nullptr;
            }

            break;
        }

        case Qt5SettingArgumentType_Blob:
        {
            if (src.dataPointer)
            {
                dst->dataPointer = calloc(dst->dataSize, 1);
                memcpy(dst->dataPointer, src.dataPointer, dst->dataSize);
            }
            else
            {
                dst->dataPointer = nullptr;
            }

            break;
        }

        default:
            break;
    }
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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

	char8 idString[256];

    sprintf(idString, "Setting%i/Argument%i/Type", settingIndex, argumentIndex);
    settings.setValue(idString, static_cast<int32>(setting->arguments[argumentIndex].type));

    sprintf(idString, "Setting%i/Argument%i/Size", settingIndex, argumentIndex);
    settings.setValue(idString, static_cast<int32>(setting->arguments[argumentIndex].dataSize));

    sprintf(idString, "Setting%i/Argument%i/Data", settingIndex, argumentIndex);

    switch(setting->arguments[argumentIndex].type)
    {
        case Qt5SettingArgumentType_Int8:
        case Qt5SettingArgumentType_Int16:
        case Qt5SettingArgumentType_Int32:
        {
            settings.setValue(idString,
            				  int32((int64)setting->arguments[argumentIndex].dataPointer));
            break;
        }

        case Qt5SettingArgumentType_UInt8:
        case Qt5SettingArgumentType_UInt16:
        case Qt5SettingArgumentType_UInt32:
        {
            settings.setValue(idString,
            				  uint32((uint64)setting->arguments[argumentIndex].dataPointer));
            break;
        }

        case Qt5SettingArgumentType_Int64:
        {
            settings.setValue(idString,
            				  *((qlonglong*)setting->arguments[argumentIndex].dataPointer));
            break;
        }

        case Qt5SettingArgumentType_UInt64:
        {
            settings.setValue(idString,
            				  *((qulonglong*)setting->arguments[argumentIndex].dataPointer));
            break;
        }

        case Qt5SettingArgumentType_Bool:
        {
            settings.setValue(idString,
            				  bool(setting->arguments[argumentIndex].dataPointer));
            break;
        }

        case Qt5SettingArgumentType_String:
        {
            if (setting->arguments[argumentIndex].dataPointer)
            {
                settings.setValue(idString,
                	              static_cast<char8*>(setting->arguments[argumentIndex].dataPointer));
            }

            break;
        }

        case Qt5SettingArgumentType_Blob:
        {
            if (setting->arguments[argumentIndex].dataPointer)
            {
                settings.setValue(idString,
                	              QByteArray(static_cast<char8*>(setting->arguments[argumentIndex].dataPointer),
                	              	         setting->arguments[argumentIndex].dataSize));
            }

            break;
        }

        default:
            break;
    }
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
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

    char8 idString[256];

    sprintf(idString, "Setting%i/Argument%i/Type", settingIndex, argumentIndex);
    setting->arguments[argumentIndex].type = static_cast<Qt5SettingArgumentType>(settings.value(idString).toInt());

    sprintf(idString, "Setting%i/Argument%i/Size", settingIndex, argumentIndex);
    setting->arguments[argumentIndex].dataSize = settings.value(idString).toInt();

    sprintf(idString, "Setting%i/Argument%i/Data", settingIndex, argumentIndex);

    switch (setting->arguments[argumentIndex].type)
    {
        case Qt5SettingArgumentType_Int8:
        case Qt5SettingArgumentType_Int16:
        case Qt5SettingArgumentType_Int32:
        {
            setting->arguments[argumentIndex].dataPointer = reinterpret_cast<int32*>(settings.value(idString).toInt());
            break;
        }

        case Qt5SettingArgumentType_UInt8:
        case Qt5SettingArgumentType_UInt16:
        case Qt5SettingArgumentType_UInt32:
        {
            setting->arguments[argumentIndex].dataPointer = reinterpret_cast<int32*>(settings.value(idString).toUInt());
            break;
        }

        case Qt5SettingArgumentType_Int64:
        {
            setting->arguments[argumentIndex].dataPointer = calloc(sizeof(int64), 1);
            qlonglong tmpLongLong = settings.value(idString).toLongLong();
            memcpy(setting->arguments[argumentIndex].dataPointer, &tmpLongLong, sizeof(int64));
            break;
        }

        case Qt5SettingArgumentType_UInt64:
        {
            setting->arguments[argumentIndex].dataPointer = calloc(sizeof(uint64), 1);
            qulonglong tmpULongLong = settings.value(idString).toULongLong();
            memcpy(setting->arguments[argumentIndex].dataPointer, &tmpULongLong, sizeof(uint64));
            break;
        }

        case Qt5SettingArgumentType_Bool:
        {
            setting->arguments[argumentIndex].dataPointer = reinterpret_cast<bool*>(settings.value(idString).toBool());
            break;
        }

        case Qt5SettingArgumentType_String:
        {
            if (setting->arguments[argumentIndex].dataPointer)
            {
				free(setting->arguments[argumentIndex].dataPointer);
			}

            QString tmpString = settings.value(idString).toString();
            if (!tmpString.isNull() && tmpString.length() > 0)
            {
                QByteArray tmpbStr = tmpString.toLatin1();
                const char8* tempStr = tmpbStr.data();
                setting->arguments[argumentIndex].dataPointer = static_cast<char8*>(calloc(strlen(tempStr) + 1, 1));
                memcpy(setting->arguments[argumentIndex].dataPointer, tempStr, strlen(tempStr) + 1);
            }
            else
            {
                setting->arguments[argumentIndex].dataPointer = nullptr;
            }

            break;
        }

        case Qt5SettingArgumentType_Blob:
        {
            if (setting->arguments[argumentIndex].dataPointer)
            {
				free(setting->arguments[argumentIndex].dataPointer);
			}

			QByteArray tmpData = settings.value(idString).toByteArray();
			if (!tmpData.isNull() && tmpData.size() > 0)
			{
				const char8* tempData = tmpData.data();
				setting->arguments[argumentIndex].dataPointer = static_cast<char8*>(calloc(setting->arguments[argumentIndex].dataSize, 1));
				memcpy(setting->arguments[argumentIndex].dataPointer, tempData, setting->arguments[argumentIndex].dataSize);
			}
			else
			{
				setting->arguments[argumentIndex].dataPointer = nullptr;
			}

            break;
        }

        default:
            break;
    }
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
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

	if (!settings.contains("Layout/EntryCount"))
	{
		layout->entryCount = 0;
		return;
	}

	char8 idString[256];

	layout->entryCount = settings.value("Layout/EntryCount").toInt();
	layout->entries = static_cast<Qt5LayoutEntry*>(malloc(sizeof(Qt5LayoutEntry) * layout->entryCount));

	for (int32 index = 0; index < layout->entryCount; ++index)
	{
		resetEntry(&layout->entries[index]);

		sprintf(idString, "Layout/Entry%i/EntryId", index);
		layout->entries[index].entryId = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/ViewType", index);
		layout->entries[index].viewType = static_cast<Qt5ViewType>(settings.value(idString).toInt());

		sprintf(idString, "Layout/Entry%i/ParentId", index);
		layout->entries[index].parentId = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/PositionX", index);
		layout->entries[index].positionX = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/PositionY", index);
		layout->entries[index].positionY = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/SizeX", index);
		layout->entries[index].sizeX = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/SizeY", index);
		layout->entries[index].sizeY = settings.value(idString).toInt();

		sprintf(idString, "Layout/Entry%i/ExtendedData", index);

        if (settings.value(idString).type() == QVariant::String)
        {
            QByteArray value = settings.value(idString).toByteArray();

            if (layout->entries[index].extendedData.dataPointer)
            {
                free(layout->entries[index].extendedData.dataPointer);
                layout->entries[index].extendedData.dataPointer = nullptr;
            }

		    layout->entries[index].extendedData.dataPointer = calloc(value.size() + 1, sizeof(char8));
            strcpy(static_cast<char8*>(layout->entries[index].extendedData.dataPointer), value.data());
            layout->entries[index].extendedData.dataSize = value.size();
            layout->entries[index].extendedData.type = Qt5SettingArgumentType_String;
        }

        if (layout->entries[index].viewType == Qt5ViewType_Main)
		{
			sprintf(idString, "Layout/Entry%i/IsMaximized", index);
			layout->entries[index].isMaximized = settings.value(idString).toBool();

			sprintf(idString, "Layout/Entry%i/MainWindowState", index);
			layout->entries[index].mainWindowState = new QByteArray(settings.value(idString).toByteArray());
		}
		else if (layout->entries[index].viewType == Qt5ViewType_Dynamic)
		{
			sprintf(idString, "Layout/Entry%i/Child1", index);
			layout->entries[index].child1 = settings.value(idString).toInt();

			sprintf(idString, "Layout/Entry%i/Child2", index);
			layout->entries[index].child2 = settings.value(idString).toInt();

			sprintf(idString, "Layout/Entry%i/IsFloating", index);
			layout->entries[index].isFloating = settings.value(idString).toBool();

			sprintf(idString, "Layout/Entry%i/HasSplitter", index);
			layout->entries[index].hasSplitter = settings.value(idString).toBool();

			sprintf(idString, "Layout/Entry%i/FillMainWindow", index);
			layout->entries[index].fillMainWindow = settings.value(idString).toBool();

			sprintf(idString, "Layout/Entry%i/TopLevel", index);
			layout->entries[index].topLevel = settings.value(idString).toBool();

			if (layout->entries[index].hasSplitter)
			{
				sprintf(idString, "Layout/Entry%i/SplitRegion1Size", index);
				layout->entries[index].splitRegion1Size = settings.value(idString).toInt();

				sprintf(idString, "Layout/Entry%i/SplitRegion2Size", index);
				layout->entries[index].splitRegion2Size = settings.value(idString).toInt();

				sprintf(idString, "Layout/Entry%i/SplitDirection", index);
				layout->entries[index].splitDirection = static_cast<Qt::Orientation>(settings.value(idString).toInt());
			}

			if (!layout->entries[index].fillMainWindow)
			{
				sprintf(idString, "Layout/Entry%i/DockPositionX", index);
				layout->entries[index].dockPositionX = settings.value(idString).toInt();

				sprintf(idString, "Layout/Entry%i/DockPositionY", index);
				layout->entries[index].dockPositionY = settings.value(idString).toInt();

				sprintf(idString, "Layout/Entry%i/DockSizeX", index);
				layout->entries[index].dockSizeX = settings.value(idString).toInt();

				sprintf(idString, "Layout/Entry%i/DockSizeY", index);
				layout->entries[index].dockSizeY = settings.value(idString).toInt();
			}
		}
	}
}

#if NcFeature(Qt5SettingsDebugLayout)
inline const char* viewTypeToString(Qt5ViewType type)
{
	switch (type)
	{
		case Qt5ViewType_Init:
			return "Init";

		case Qt5ViewType_Reset:
			return "Reset";

		case Qt5ViewType_Dock:
			return "Dock";

		case Qt5ViewType_Main:
			return "Main";

		case Qt5ViewType_Dynamic:
			return "Dynamic";

		case Qt5ViewType_CallStack:
			return "CallStack";

		case Qt5ViewType_Locals:
			return "Locals";

		case Qt5ViewType_SourceCode:
			return "Source";

		case Qt5ViewType_HexEdit:
			return "Memory";

		case Qt5ViewType_DebugOutput:
			return "DebugOutput";

		case Qt5ViewType_Registers:
			return "Registers";

		default:
			return type >= Qt5ViewType_PluginStart ? "Unknown Plugin" : "Unknown";
	}
}

void Qt5Settings::debugLayout(Qt5Layout* layout)
{
	for (int32 index = 0; index < layout->entryCount; ++index)
	{
		printf("View[%d]\n",     index);
		printf("\tId: %d\n",     layout->entries[index].entryId);
		printf("\tType: %s\n",   viewTypeToString(layout->entries[index].viewType));
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