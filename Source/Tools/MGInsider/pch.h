#pragma once

#include <Config/pch_common.h>

#pragma warning (disable: 4251)

#pragma warning (push, 0)
#include <qapplication.h>
#include <qobject.h>
#include <QtWidgets/QMainWindow>
#include <qdialog.h>
#include <qtextdocumentwriter.h>
#include <qfiledialog.h>
#include <QPainter>
#include <QPlainTextEdit>
#include <qfileinfo.h>
#include <QTabWidget.h>
#include <qboxlayout.h>
#include <QTextBlock>
#include <QStandardItemModel.h>
#include <qmessagebox.h>
#include <qmenu.h>
#include <QVariant>
#include <qtimer.h>
#include <qfont.h>
#include <qlist.h>
#include <qevent.h>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QVector>
#include <qudpsocket.h>
#include <qinputdialog.h>
#include <qclipboard.h>
#include <QThread>
#pragma warning ( pop )

#include <OrbitLogger/OrbitLogger.h>

#include <cassert>

#include <pugixml-1.6/src/pugixml.hpp>
#include <luaJiT/lua.hpp>

#include <source/XMLUtils.h>
#include <source/Utils/SetGet.h>
#include "Utils/Memory/nMemory.h"
#include "Utils/Memory.h"
#include <source/Utils/LuaUtils.h>

#include <GabiLib/src/utils/EnumConverter.h>
#include <GabiLib/src/rtti/RTTI.h>
#include <GabiLib/src/rtti/DynamicClassRegister.h>
