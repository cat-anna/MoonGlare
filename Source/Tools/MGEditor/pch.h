#pragma once

#include <Config/pch_common.h>

#pragma warning(disable : 4251)
#pragma warning(disable : 4290)

#pragma warning(push, 0)

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QInputDialog>
#include <QMenu>
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
#include <qevent.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qinputdialog.h>
#include <qlist.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qmessagebox>
#include <qobject.h>
#include <qtextdocumentwriter.h>
#include <qtimer.h>
#include <qudpsocket.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <orbit_logger.h>

#include <cassert>

#include <lua.hpp>
#include <pugixml.hpp>

#include "Foundation/Memory/Memory.h"
#include "Foundation/Memory/nMemory.h"
#include <Foundation/Flags.h>
#include <Foundation/XMLUtils.h>
#include <Math/libMath.h>

#include <dynamic_class_register.h>
#include <libSpace/src/Memory/Handle.h>
#include <libSpace/src/RTTI/RTTI.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
#include <libSpace/src/Utils/EnumConverter.h>
using namespace Space::Utils::HashLiterals;

#include <Engine/Configuration.h>

#include <boost/filesystem.hpp>
