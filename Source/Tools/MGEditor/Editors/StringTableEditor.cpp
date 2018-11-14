#include PCH_HEADER
#include <ui_StringTableEditor.h>
#include "StringTableEditor.h"

#include <fmt/format.h>

#include <boost/algorithm/string.hpp>

namespace MoonGlare::Editor {

class EmptyCellDelegate : public QStyledItemDelegate {
public:
    EmptyCellDelegate::EmptyCellDelegate() {
    }

    void EmptyCellDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        bool e = index.data(Qt::DisplayRole).toString().isEmpty();

        if (e) {
            painter->fillRect(option.rect, QColor(Qt::darkRed));
        }

        QStyledItemDelegate::paint(painter, option, index);
    }
};

//-------------------------------------------------------------------------------------------------

StringTableEditor::StringTableEditor(QWidget * parent, SharedModuleManager smm, std::string tableName)
    :  QWidget(parent), iChangeContainer(smm), sharedModuleManager(std::move(smm)), tableName(std::move(tableName)) {
    ui = std::make_unique<Ui::StringTableEditor>();
    ui->setupUi(this);

    SetChangesName("String table " + this->tableName);

    fileSystem = sharedModuleManager->QuerryModule<FileSystem>();
    assert(fileSystem);

    FileInfoTable fit;
    fileSystem->EnumerateFolder("/Tables", fit, false);

    languages.emplace_back(LangInfo{ "default" , "" , 1, true});

    std::set<std::string> langs;
    for (const auto &item : fit) {
        std::vector<std::string> parts;
        boost::split(parts, item.m_RelativeFileName, boost::is_any_of("."));

        bool exists = true;
        if (parts[0] != this->tableName)
            exists = false;
            
        if (parts.size() == 3) {
            if (langs.find(parts[1]) == langs.end()) {
                langs.insert(parts[1]);
                languages.emplace_back(LangInfo{ parts[1], "." + parts[1], (int)languages.size() + 1 , exists });
            }
        }
    }     

    itemModel = std::make_unique<QStandardItemModel>();
    connect(itemModel.get(), &QStandardItemModel::itemChanged, [this](QStandardItem * item) {
        SetModiffiedState(true);
    });

    ui->treeView->setModel(itemModel.get());

    itemModel->setHorizontalHeaderItem(0, new QStandardItem("Id"));
    int hdr = 1;
    for (auto &item : languages) {
        itemModel->setHorizontalHeaderItem(hdr, new QStandardItem(item.title.c_str()));
        ui->treeView->setColumnWidth(hdr, 250);
        ++hdr;
    }

    ui->treeView->setColumnWidth(0, 250);
    ui->treeView->setItemDelegate(new EmptyCellDelegate());

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, [this](const QPoint &pos) {
        QMenu menu(this);
        bool selection = ui->treeView->currentIndex().isValid();

        menu.addAction("Add row", [this]() {
            QList<QStandardItem*> cols;
            cols << new QStandardItem();
            for (auto &l : languages)
                cols << new QStandardItem("");
            auto root = itemModel->invisibleRootItem();
            root->appendRow(cols);
            SetModiffiedState(true);
        });

        menu.addSeparator();

        menu.addAction("Remove row", [this]() {
            int row = ui->treeView->currentIndex().row();
            itemModel->removeRow(row);
            SetModiffiedState(true);
        })->setEnabled(selection);

        menu.exec(QCursor::pos());
    });

    Reload();
}

StringTableEditor::~StringTableEditor() {

}

//-------------------------------------------------------------------------------------------------

void StringTableEditor::Reload() {
    //itemModel->clear();
    auto root = itemModel->invisibleRootItem();
    std::unordered_map<std::string, QStandardItem*> idToRow;

    for (const auto &li : languages) {
        if (!li.exists)
            continue;

        XMLFile xfile;
        auto tfname = "file:///Tables/" + tableName + li.suffix + ".xml";
        if (!fileSystem->OpenXML(xfile, tfname)) {
            __debugbreak();
            return;
        }

        for (pugi::xml_node node = xfile->document_element().first_child(); node; node = node.next_sibling()) {
            std::string id = node.attribute("Id").as_string();
            std::string value = node.text().as_string();

            auto &row = idToRow[id];
            if (!row) {
                QList<QStandardItem*> cols;
                cols << (row = new QStandardItem(id.c_str()));
                for (auto &l : languages)
                    cols << new QStandardItem("");

                auto cell = cols.at(li.column);
                cell->setText(value.c_str());
                if (value.empty())
                    cell->setBackground(Qt::darkYellow);
                root->appendRow(cols);
            } else {
                auto cell = root->child(row->row(), li.column);
                cell->setText(value.c_str());
                if (value.empty())
                    cell->setBackground(Qt::darkYellow);
            }
        }
    }

    root->sortChildren(0);
    SetModiffiedState(false);
}

//-------------------------------------------------------------------------------------------------

bool StringTableEditor::SaveChanges() {
    auto root = itemModel->invisibleRootItem();

    for (auto &item : languages) {
        pugi::xml_document xdoc;
        auto node = xdoc.append_child("StringTable");
        //root.append_attribute("lang") = ...

        for (size_t i = 0; i < root->rowCount(); ++i) {
            auto idItm = root->child(i, 0);
            auto colItm = root->child(i, item.column);

            std::string id = idItm->data(Qt::DisplayRole).toString().toUtf8().constData();
            std::string value = colItm->data(Qt::DisplayRole).toString().toUtf8().constData();
            if (!value.empty())
                item.exists = true;
            auto n = node.append_child("Item");
            n.append_attribute("Id") = id.c_str();
            n.text().set(value.c_str());
        }

        if (!item.exists)
            continue;

        std::stringstream ss;
        xdoc.save(ss);
        std::string fname = "file:///Tables/" + tableName + item.suffix + ".xml";

        StarVFS::ByteTable bt;
        bt.from_string(ss.str());
        fileSystem->SetFileData(fname, bt);
    }

    SetModiffiedState(false);
    return true;
}     
                  
//-------------------------------------------------------------------------------------------------

std::string StringTableEditor::GetTabTitle() const {
    return "String table " + tableName;
}

bool StringTableEditor::CanClose() const {
    if (isWindowModified()) {

    }
    return true;
}



} 

