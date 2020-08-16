#pragma once

#include <Config/pch_common.h>

#pragma warning(disable : 4251)

#pragma warning(push, 0)

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QInputDialog>
#include <QPainter>
#include <QPlainTextEdit>
#include <QRegExp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSyntaxHighlighter>
#include <QTabWidget.h>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QThread>
#include <QVariant>
#include <QVector>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMainWindow>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qclipboard.h>
#include <qdialog.h>
#include <qdockwidget.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qinputdialog.h>
#include <qlist.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qtextdocumentwriter.h>
#include <qtimer.h>
#include <qudpsocket.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#pragma warning(pop)

#include <orbit_logger.h>

#include <cassert>

#include <luaJiT-2.0.4/lua.hpp>
#include <pugixml-1.6/src/pugixml.hpp>

#include "Utils/Memory.h"
#include "Utils/Memory/nMemory.h"
#include <source/Utils/LuaUtils.h>
#include <source/Utils/SetGet.h>
#include <source/Utils/XMLUtils.h>

#include <dynamic_class_register.h>
#include <libSpace/src/Memory/Handle.h>
#include <libSpace/src/RTTI/RTTI.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
#include <libSpace/src/Utils/EnumConverter.h>
using namespace Space::Utils::HashLiterals;

#include <Engine/Configuration.h>

namespace MoonGlare {}
using namespace MoonGlare;

Q_DECLARE_METATYPE(MoonGlare::Handle);
