#pragma once	

#include <Config/pch_common.h>

#pragma warning (disable: 4251)
#pragma warning( disable : 4290 )  

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
#include <QtWidgets/QDockWidget>
#include <QTextBlock>
#include <QStandardItemModel>
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
#include <QMenu>
#include <qmessagebox>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QInputDialog>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>

inline std::string ToLower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return std::move(str);
}

#pragma warning ( pop )


#include <OrbitLogger/src/OrbitLogger.h>

#include <cassert>

#include <pugixml.hpp>
#include <lua.hpp>

#include <Foundation/XMLUtils.h>
#include <Foundation/Flags.h>
#include "Foundation/Memory/nMemory.h"
#include "Foundation/Memory.h"
#include <Foundation/xMath.h>

#include <libSpace/src/Utils/EnumConverter.h>
#include <libSpace/src/RTTI/RTTI.h>
#include <libSpace/src/Utils/DynamicClassRegister.h>
#include <libSpace/src/Memory/Handle.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
using namespace Space::Utils::HashLiterals;

#include <Engine/Configuration.h>

#include <boost/filesystem.hpp>
