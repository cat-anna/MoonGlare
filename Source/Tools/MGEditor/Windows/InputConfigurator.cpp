#include PCH_HEADER
#include <qobject.h>
#include <pugixml.hpp>
#include <GLFW/glfw3.h>
#include "InputConfigurator.h"
#include "ui_InputConfigurator.h"
#include <boost/algorithm/string.hpp>

#include <qtUtils.h>

#include <Foundation/InputProcessor.h>
#include <Input.x2c.h>

namespace MoonGlare {

const Core::KeyNamesTable Core::g_KeyNamesTable;

namespace Editor {      

struct QtRoles {
    enum {
        InputName = Qt::UserRole + 1000,
        ModifyMode,
    };
};

enum class ModifyMode {
    Invalid,
    Name,
    Type,
    Key,
    PositiveKey,
    NegativeKey,
    Sensivity,
    MouseAxis,
};
Q_DECLARE_METATYPE(MoonGlare::Editor::ModifyMode);

Q_DECLARE_METATYPE(Core::MouseAxisId);

//----------------------------------------------------------------------------------

struct InputConfigurator::DataModel {

    using KeyId = ::MoonGlare::x2c::Core::Input::KeyCode;
    using KeyCodeVector = ::MoonGlare::x2c::Core::Input::KeyCodeVector;
    using InputConfiguration_t = x2c::Core::Input::InputConfiguration_t;

    struct InputSwitch {
        KeyCodeVector keys = { 0 };

        auto Add(InputConfiguration_t &config) -> decltype(config.m_KeyboardSwitches)::value_type&{
            decltype(config.m_KeyboardSwitches)::value_type item;
            item.m_Keys = keys;
            config.m_KeyboardSwitches.push_back(item);
            return config.m_KeyboardSwitches.back();
        }

        void SetView(QStandardItem *item) const {
            for (auto k : keys) {
                QList<QStandardItem*> cols;
                cols << new QStandardItem("Key");
                cols << new QStandardItem();
                auto e = new QStandardItem(fmt::format("{} ({})", Core::g_KeyNamesTable[k], k).c_str());
                e->setData(QVariant::fromValue(ModifyMode::Key), QtRoles::ModifyMode);
                cols << e;
                item->appendRow(cols);
            }
        };

        static constexpr char TypeName[] = "Switch";
    };

    struct InputSwitchAxis {
        KeyCodeVector positiveKeys = { 0 };
        KeyCodeVector negativeKeys = { 0 };

        auto Add(InputConfiguration_t &config) -> decltype(config.m_KeyboardAxes)::value_type&{
            decltype(config.m_KeyboardAxes)::value_type item;
            item.m_NegativeKeys = negativeKeys;
            item.m_PositiveKeys = positiveKeys;
            config.m_KeyboardAxes.push_back(item);
            return config.m_KeyboardAxes.back();
        }

        void SetView(QStandardItem *item) const {
            for (auto k : positiveKeys) {
                QList<QStandardItem*> cols;
                cols << new QStandardItem("Positive Key");
                cols << new QStandardItem();
                auto e = new QStandardItem(fmt::format("{} ({})", Core::g_KeyNamesTable[k], k).c_str());
                e->setData(QVariant::fromValue(ModifyMode::PositiveKey), QtRoles::ModifyMode);
                cols << e;
                item->appendRow(cols);
            }
            for (auto k : negativeKeys) {
                QList<QStandardItem*> cols;
                cols << new QStandardItem("Negative Key");
                cols << new QStandardItem();
                auto e = new QStandardItem(fmt::format("{} ({})", Core::g_KeyNamesTable[k], k).c_str());
                e->setData(QVariant::fromValue(ModifyMode::NegativeKey), QtRoles::ModifyMode);
                cols << e;
                item->appendRow(cols);
            }
        };

        static constexpr char TypeName[] = "Switch axis";
    };

    struct InputMouseAxis {
        Core::MouseAxisId axisId = Core::MouseAxisId::Unknown;
        float sensivity = { 0.5f };

        auto Add(InputConfiguration_t &config) -> decltype(config.m_MouseAxes)::value_type&{
            decltype(config.m_MouseAxes)::value_type item;
            item.m_AxisId = axisId;
            item.m_Sensitivity = sensivity;
            config.m_MouseAxes.push_back(item);
            return config.m_MouseAxes.back();
        }

        void SetView(QStandardItem *item) const {
            QList<QStandardItem*> cols;
            cols << new QStandardItem("Axis");
            cols << new QStandardItem();
            auto e = new QStandardItem(fmt::format("{}", Core::g_KeyNamesTable[axisId]).c_str());
            e->setData(QVariant::fromValue(ModifyMode::MouseAxis), QtRoles::ModifyMode);
            cols << e;
            item->appendRow(cols);

            QList<QStandardItem*> cols2;
            cols2 << new QStandardItem("Sensitivity");
            cols2 << new QStandardItem();
            auto e2 = new QStandardItem(std::to_string(sensivity).c_str());
            e2->setData(QVariant::fromValue(ModifyMode::Sensivity), QtRoles::ModifyMode);
            cols2 << e2;
            item->appendRow(cols2);
        };

        static constexpr char TypeName[] = "Mouse axis";
    };

    struct Input {
        std::variant<InputSwitch, InputSwitchAxis, InputMouseAxis> config;
    };

    std::unordered_map<std::string, Input> inputs;

    DataModel() {
        SetKeyboardAxis("Forward", (KeyId)GLFW_KEY_W, (KeyId)GLFW_KEY_S);
        SetKeyboardAxis("SideStep", (KeyId)GLFW_KEY_A, (KeyId)GLFW_KEY_D);
        SetKeyboardSwitch("Run", (KeyId)GLFW_KEY_LEFT_SHIFT);
        SetMouseAxis("LookAngle", Core::MouseAxisId::Y, 0.5f);
        SetMouseAxis("Turn", Core::MouseAxisId::X, 0.5f);
    }

    void AddNew() {
        for (int i = 1; i < 100; ++i) {
            auto name = fmt::format("NewSwitch_{}", i);
            if (inputs.find(name) == inputs.end()) {
                SetKeyboardSwitch(name);
                return;
            }
        }
    }

    void Remove(const std::string& Name) {
        auto it = inputs.find(Name);
        if (it == inputs.end())
            return;
        inputs.erase(it);
    }

    void Rename(const std::string& oldName, std::string newName) {
        auto it = inputs.find(oldName);
        if(it == inputs.end())
            return;

        boost::replace_all(newName, " ", "_");

        if (inputs.find(newName) != inputs.end())
            return;

        inputs[newName] = it->second;
        inputs.erase(it);
    }

    Input& Get(const std::string &Name) {
        return inputs[Name];
    }

    void SetKeyboardAxis(const std::string& name, std::optional<KeyId> positive = std::nullopt, std::optional<KeyId> negative = std::nullopt) {
        auto &input = inputs[name];
        InputSwitchAxis isa;
        if (std::holds_alternative<InputSwitchAxis>(input.config))
            isa = std::get<InputSwitchAxis>(input.config);
        if (positive.has_value())
            isa.positiveKeys = { *positive };
        if (negative.has_value())
            isa.negativeKeys = { *negative };
        input.config = isa;
    }

    void SetKeyboardSwitch(const std::string& name, std::optional<KeyId> key = std::nullopt) {
        auto &input = inputs[name];
        InputSwitch is;
        if (std::holds_alternative<InputSwitch>(input.config))
            is = std::get<InputSwitch>(input.config);
        if (key.has_value())
            is.keys = { *key };
        input.config = is;
    }

    void SetMouseAxis(const std::string& name, std::optional<Core::MouseAxisId> axis = std::nullopt, std::optional<float> sensivity = std::nullopt) {
        auto &input = inputs[name];
        InputMouseAxis ima;
        if (std::holds_alternative<InputMouseAxis>(input.config))
            ima = std::get<InputMouseAxis>(input.config);
        if (sensivity.has_value())
            ima.sensivity = { *sensivity };
        if (axis.has_value())
            ima.axisId = *axis;
        input.config = ima;
    }

    void Read(const InputConfiguration_t &config) {
        inputs.clear();
        for (auto &it : config.m_KeyboardSwitches) {
            InputSwitch is;
            is.keys = it.m_Keys;

            Input i;
            i.config = is;
            inputs[it.m_Name] = i;
        }

        for (auto &it : config.m_KeyboardAxes) {
            InputSwitchAxis isa;
            isa.negativeKeys = it.m_NegativeKeys;
            isa.positiveKeys = it.m_PositiveKeys;

            Input i;
            i.config = isa;
            inputs[it.m_Name] = i;
        }

        for (auto &it : config.m_MouseAxes) {
            InputMouseAxis ima;
            ima.axisId = it.m_AxisId;
            ima.sensivity = it.m_Sensitivity;

            Input i;
            i.config = ima;
            inputs[it.m_Name] = i;
        }
    }

    void Write(InputConfiguration_t &config) {
        config.m_KeyboardSwitches.clear();
        config.m_KeyboardAxes.clear();
        config.m_MouseAxes.clear();

        for (auto &input : inputs) {
            std::visit([&](auto&& arg) {
                auto &item = arg.Add(config);
                item.m_Name = input.first;
            }, input.second.config);
        }
    }
};

//----------------------------------------------------------------------------------

struct InputConfigurator::CustomItemDelegate : public QStyledItemDelegate {
    using KeyId = ::MoonGlare::x2c::Core::Input::KeyCode;

    CustomItemDelegate(InputConfigurator *parent, DataModel *dataModel): QStyledItemDelegate(parent), owner(parent), dataModel(dataModel){ }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        //auto vinfo = index.data(QtRoles::StructureValue).value<StructureValue*>();
        auto mode = index.data(QtRoles::ModifyMode).value<ModifyMode>();
        switch (mode) {
        case ModifyMode::Name:
            return QStyledItemDelegate::createEditor(parent, option, index);
        case ModifyMode::Type: {
            QComboBox *cb = new QComboBox(parent);
            cb->addItem(DataModel::InputSwitch::TypeName, QVariant::fromValue(1));
            cb->addItem(DataModel::InputSwitchAxis::TypeName, QVariant::fromValue(2));
            cb->addItem(DataModel::InputMouseAxis::TypeName, QVariant::fromValue(3));
            cb->setInsertPolicy(QComboBox::NoInsert);
            cb->model()->sort(0);
            return cb;                 
        }
        case ModifyMode::Sensivity: {
            QDoubleSpinBox *dsp = new QDoubleSpinBox(parent);
            dsp->setSingleStep(0.1);
            dsp->setMinimum(-1.0f);
            dsp->setMaximum(1.0f);
            dsp->setDecimals(1);
            return dsp;
        }
        case ModifyMode::Key:
        case ModifyMode::PositiveKey:
        case ModifyMode::NegativeKey: {
            QComboBox *cb = new QComboBox(parent);
            auto &t = Core::g_KeyNamesTable.GetTable();
            for (KeyId i = 0; i < (KeyId)t.size(); ++i) {
                if(t[i])
                    cb->addItem(t[i], QVariant::fromValue(i));
            }
            cb->setInsertPolicy(QComboBox::NoInsert);
            return cb;
        }
        case ModifyMode::MouseAxis: {
            QComboBox *cb = new QComboBox(parent);
            cb->addItem(Core::g_KeyNamesTable[Core::MouseAxisId::X], QVariant::fromValue((int)Core::MouseAxisId::X));
            cb->addItem(Core::g_KeyNamesTable[Core::MouseAxisId::Y], QVariant::fromValue((int)Core::MouseAxisId::Y));
            cb->addItem(Core::g_KeyNamesTable[Core::MouseAxisId::ScrollX], QVariant::fromValue((int)Core::MouseAxisId::ScrollX));
            cb->addItem(Core::g_KeyNamesTable[Core::MouseAxisId::ScrollY], QVariant::fromValue((int)Core::MouseAxisId::ScrollY));
            cb->setInsertPolicy(QComboBox::NoInsert);
            cb->model()->sort(0);
            return cb;
        }
        default:
            return nullptr;
        }
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        auto row = index.row();
        auto parent = index.parent();
        auto selectedindex = parent.isValid() ? parent : index.sibling(row, 0);
        auto itemptr = ((QStandardItemModel*)index.model())->itemFromIndex(selectedindex);
        auto mode = index.data(QtRoles::ModifyMode).value<ModifyMode>();
        std::string Name = itemptr->data(Qt::DisplayRole).value<QString>().toLocal8Bit().constData();

        using InputMouseAxis = DataModel::InputMouseAxis;
        using InputSwitch = DataModel::InputSwitch;
        using InputSwitchAxis = DataModel::InputSwitchAxis;

        auto &config = dataModel->Get(Name).config;
        switch (mode) {
        case ModifyMode::Name:
            ((QLineEdit*)editor)->setText(index.data(Qt::DisplayRole).value<QString>());
            break;
        case ModifyMode::Type:
            ((QComboBox*)editor)->setCurrentText(index.data(Qt::DisplayRole).value<QString>());
            break;
        case ModifyMode::Sensivity:
            ((QDoubleSpinBox*)editor)->setValue(std::get<InputMouseAxis>(config).sensivity);
            break;
        case ModifyMode::Key:
            ((QComboBox*)editor)->setCurrentText(Core::g_KeyNamesTable[std::get<InputSwitch>(config).keys[0]]);
            break;
        case ModifyMode::PositiveKey:
            ((QComboBox*)editor)->setCurrentText(Core::g_KeyNamesTable[std::get<InputSwitchAxis>(config).positiveKeys[0]]);
            break;
        case ModifyMode::NegativeKey:
            ((QComboBox*)editor)->setCurrentText(Core::g_KeyNamesTable[std::get<InputSwitchAxis>(config).negativeKeys[0]]);
            break;
        case ModifyMode::MouseAxis:
            ((QComboBox*)editor)->setCurrentText(Core::g_KeyNamesTable[std::get<InputMouseAxis>(config).axisId]);
            break;
        default:
            break;
        }
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
        auto row = index.row();
        auto parent = index.parent();
        auto selectedindex = parent.isValid() ? parent : index.sibling(row, 0);
        auto itemptr = ((QStandardItemModel*)model)->itemFromIndex(selectedindex);
        auto mode = index.data(QtRoles::ModifyMode).value<ModifyMode>();
        std::string Name = itemptr->data(Qt::DisplayRole).value<QString>().toLocal8Bit().constData();

        switch (mode) {
        case ModifyMode::Name: {
            std::string newName = ((QLineEdit*)editor)->text().toLocal8Bit().constData();
            dataModel->Rename(Name, newName);
            break;
        }
        case ModifyMode::Type: {
            std::string newType = ((QComboBox*)editor)->currentText().toLocal8Bit().constData();
            if (newType == DataModel::InputSwitch::TypeName)
                dataModel->SetKeyboardSwitch(Name);
            else if (newType == DataModel::InputSwitchAxis::TypeName)
                dataModel->SetKeyboardAxis(Name);
            else if (newType == DataModel::InputMouseAxis::TypeName)
                dataModel->SetMouseAxis(Name);
            break;
        }
        case ModifyMode::Sensivity: {
            auto newValue = static_cast<float>(((QDoubleSpinBox*)editor)->value());
            dataModel->SetMouseAxis(Name, std::nullopt, newValue);
            break;
        }
        case ModifyMode::Key: {
            auto newKey = ((QComboBox*)editor)->currentData().value<KeyId>();
            dataModel->SetKeyboardSwitch(Name, newKey);
            break;
        }
        case ModifyMode::PositiveKey: {
            auto newKey = ((QComboBox*)editor)->currentData().value<KeyId>();
            dataModel->SetKeyboardAxis(Name, newKey);
            break;
        }
        case ModifyMode::NegativeKey: {
            auto newKey = ((QComboBox*)editor)->currentData().value<KeyId>();
            dataModel->SetKeyboardAxis(Name, std::nullopt, newKey);
            break;
        }
        case ModifyMode::MouseAxis: {
            auto newAxis = ((QComboBox*)editor)->currentData().value<Core::MouseAxisId>();
            dataModel->SetMouseAxis(Name, newAxis, std::nullopt);
            break;
        }
        default:
            break;
        }
        owner->Refresh();
    }
private:
    InputConfigurator *owner;
    DataModel *dataModel;
};

//----------------------------------------------------------------------------------

InputConfigurator::InputConfigurator(QWidget *parent, SharedModuleManager ModuleManager, std::string configFileName)
    : SubDialog(parent, ModuleManager), configFileName(std::move(configFileName))
{
    ui = std::make_unique<Ui::InputConfigurator>();
    ui->setupUi(this);

    dataModel = std::make_unique<DataModel>();

    connect(ui->pushButtonOk, &QAbstractButton::clicked, this, &InputConfigurator::AcceptChanges);
    connect(ui->pushButtonCancel, &QAbstractButton::clicked, this, &InputConfigurator::reject);

    connect(ui->actionAdd, &QAction::triggered, this, &InputConfigurator::InputAdd);
    connect(ui->actionRemove, &QAction::triggered, this, &InputConfigurator::InputRemove);

    model = std::make_unique<QStandardItemModel>();
    model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(1, new QStandardItem("Type"));
    model->setHorizontalHeaderItem(2, new QStandardItem("Key name/value"));

    ui->treeView->setModel(model.get());
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setColumnWidth(0, 150);
    ui->treeView->setColumnWidth(1, 200);
    ui->treeView->setColumnWidth(2, 200);
    ui->treeView->setItemDelegate(new CustomItemDelegate(this, dataModel.get()));

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &InputConfigurator::ShowContextMenu);

    Load();
}

InputConfigurator::~InputConfigurator() {
    ui.reset();
}

void InputConfigurator::ShowContextMenu(const QPoint &point) {
}   

void InputConfigurator::ItemDoubleClicked(const QModelIndex&) {
}

void InputConfigurator::AcceptChanges() {
    Save();
    accept();
    close();
}

void InputConfigurator::Load() {
    if (!boost::filesystem::is_regular_file(configFileName)) {
        LoadDefaultConfig();
        return;
    }

    pugi::xml_document xdoc;
    xdoc.load_file(configFileName.c_str());

    auto node = xdoc.document_element();
    x2c::Core::Input::InputConfiguration_t conf;
    if (!conf.Read(node)) {
        AddLog(Error, "Failed to read input configuration!");
        ErrorMessage("Cannot read input configuration");
        return;
    }
    dataModel->Read(conf);
    Refresh();
}

void InputConfigurator::Save() {
    pugi::xml_document xdoc;
    auto node = xdoc.append_child("InputSettings");
    x2c::Core::Input::InputConfiguration_t conf;
    dataModel->Write(conf);
    conf.Write(node);
    xdoc.save_file(configFileName.c_str());
}

void InputConfigurator::LoadDefaultConfig() {
    dataModel = std::make_unique<DataModel>();
    Refresh();
}

void InputConfigurator::Refresh() {
    model->removeRows(0, model->rowCount());
    auto root = model->invisibleRootItem();

    for (auto& item : dataModel->inputs) {
        auto&[name, input] = item;

        QStandardItem *elemName = new QStandardItem(name.c_str());
        QStandardItem *elemType = new QStandardItem(std::visit([](auto && arg) { return arg.TypeName; }, input.config));

        elemName->setData(name.c_str(), QtRoles::InputName);
        elemName->setData(QVariant::fromValue(ModifyMode::Name), QtRoles::ModifyMode);
        elemType->setData(QVariant::fromValue(ModifyMode::Type), QtRoles::ModifyMode);

        QList<QStandardItem*> cols;
        cols << elemName;
        cols << elemType;
        root->appendRow(cols);

        std::visit([&](auto && arg) { arg.SetView(elemName); }, input.config);
    }
    ui->treeView->expandAll();
}

void InputConfigurator::InputAdd() {
    dataModel->AddNew();
    Refresh();
}

void InputConfigurator::InputRemove() {
    auto index = ui->treeView->currentIndex();
    if (!index.isValid())
        return;

    auto row = index.row();
    auto parent = index.parent();
    auto selectedindex = parent.isValid() ? parent : index.sibling(row, 0);
    auto itemptr = model->itemFromIndex(selectedindex);
    std::string Name = itemptr->data(Qt::DisplayRole).value<QString>().toLocal8Bit().constData();

    dataModel->Remove(Name);
    Refresh();
}

//----------------------------------------------------------------------------------

} //namespace Editor
} //namespace MoonGlare
