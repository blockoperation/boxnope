// Copyright (c) 2016, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#include "menu.hpp"

#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlStreamReader>

#include "utils.hpp"

namespace
{

enum class ActionType
{
    None,
    System,
    Detach,
    Exit
};

ActionType getActionType(QString s)
{
    if (s == QL1S("system"))
        return ActionType::System;
    else if (s == QL1S("detach"))
        return ActionType::Detach;
    else if (s == QL1S("exit"))
        return ActionType::Exit;

    return ActionType::None;
}

struct Action
{
    ActionType type;
    QString name;
    QIcon icon;
    QString cmd;
    QStringList args;
    bool confirm;
};

inline bool confirmAction(QString name)
{
    return (QMessageBox::question(nullptr, name, QSL("Confirm?")) == QMessageBox::Yes);
}

void addMenuAction(QApplication* app, QMenu* menu, const Action& act)
{
    QAction* qact = new QAction(act.icon, act.name, menu);

    switch (act.type)
    {
        case ActionType::System:
            QObject::connect(qact, &QAction::triggered, [act]()
            {
                if (!act.confirm || confirmAction(act.name))
                {
                    QProcess p;
                    p.start(act.cmd, act.args);
                    p.waitForFinished();
                }
            });
            break;

        case ActionType::Detach:
            QObject::connect(qact, &QAction::triggered, [act]()
            {
                if (!act.confirm || confirmAction(act.name))
                    QProcess::startDetached(act.cmd, act.args);
            });
            break;

        case ActionType::Exit:
            QObject::connect(qact, &QAction::triggered, [act, app]()
            {
                if (!act.confirm || confirmAction(act.name))
                    app->quit();
            });
            break;

        default:
            break;
    }

    menu->addAction(qact);
}

enum class TagType
{
    None,
    Boxnope,
    Menu,
    Separator,
    Action,
    Argument
};

TagType getTagType(QString s)
{
    if (s == QL1S("boxnope_menu"))
        return TagType::Boxnope;
    else if (s == QL1S("menu"))
        return TagType::Menu;
    else if (s == QL1S("separator"))
        return TagType::Separator;
    else if (s == QL1S("action"))
        return TagType::Action;
    else if (s == QL1S("argument"))
        return TagType::Argument;

    return TagType::None;
}

inline QString attrToStr(const QXmlStreamAttributes& attrs, QLatin1String name)
{
    return attrs.value(name).toString();
}

inline bool attrToBool(QString s)
{
    return (s == QL1S("true") || s == QL1S("1"));
}

class MenuParser
{
public:
    MenuParser(QApplication* app, QWidget* parent, const QByteArray& data);

    QMenu* parse();

private:
    QApplication* app_;
    QWidget* parent_;
    QXmlStreamReader xml_;
    std::vector<QMenu*> menus_;
    TagType curr_tag_;
    Action curr_action_;

    void openTag();
    void closeTag();
};

MenuParser::MenuParser(QApplication* app, QWidget* parent, const QByteArray& data)
:   app_(app),
    parent_(parent),
    xml_(data),
    menus_(),
    curr_tag_(TagType::None),
    curr_action_()
{
    menus_.reserve(4);
}

QMenu* MenuParser::parse()
{
    while (!xml_.atEnd() && !xml_.hasError())
    {
        switch (xml_.readNext())
        {
            case QXmlStreamReader::StartElement:
                openTag();
                break;

            case QXmlStreamReader::EndElement:
                closeTag();
                break;

            default:
                break;
        }
    }

    return menus_.empty() ? nullptr : menus_.front();
}

void MenuParser::openTag()
{
    QXmlStreamAttributes attrs = xml_.attributes();

    switch (getTagType(xml_.name().toString()))
    {
        case TagType::Boxnope:
            if (curr_tag_ != TagType::None || !menus_.empty())
            {
                xml_.skipCurrentElement();
                break;
            }

            curr_tag_ = TagType::Boxnope;
            break;

        case TagType::Menu:
            if (curr_tag_ == TagType::Boxnope && menus_.empty())
            {
                menus_.push_back(new QMenu(attrToStr(attrs, QL1S("name")), parent_));
            }
            else if (curr_tag_ == TagType::Menu && !menus_.empty())
            {
                menus_.push_back(new QMenu(attrToStr(attrs, QL1S("name")), menus_.back()));
            }
            else
            {
                xml_.skipCurrentElement();
                break;
            }

            menus_.back()->setIcon(QIcon::fromTheme(attrToStr(attrs, QL1S("icon"))));
            curr_tag_ = TagType::Menu;
            break;

        case TagType::Separator:
            if (curr_tag_ == TagType::Menu)
            {
                if (attrs.hasAttribute(QL1S("name")))
                    menus_.back()->addSection(attrToStr(attrs, QL1S("name")));
                else
                    menus_.back()->addSeparator();
            }

            xml_.skipCurrentElement();
            break;

        case TagType::Action:
            if (curr_tag_ != TagType::Menu ||
                getActionType(attrToStr(attrs, QL1S("type"))) == ActionType::None)
            {
                xml_.skipCurrentElement();
                break;
            }

            curr_action_ = {
                getActionType(attrToStr(attrs, QL1S("type"))),
                attrToStr(attrs, QL1S("name")),
                QIcon::fromTheme(attrToStr(attrs, QL1S("icon"))),
                attrToStr(attrs, QL1S("command")),
                {},
                attrToBool(attrToStr(attrs, QL1S("confirm")))
            };
            curr_tag_ = TagType::Action;
            break;

        case TagType::Argument:
            if (curr_tag_ == TagType::Action)
                curr_action_.args << attrToStr(attrs, QL1S("value"));

            xml_.skipCurrentElement();
            break;

        default:
            xml_.skipCurrentElement();
            break;
    }
}

void MenuParser::closeTag()
{
    TagType tag = getTagType(xml_.name().toString());

    if (tag != curr_tag_)
        return;

    switch (tag)
    {
        case TagType::Boxnope:
            curr_tag_ = TagType::None;
            break;

        case TagType::Menu:
            if (menus_.size() > 1)
            {
                menus_.rbegin()[1]->addMenu(menus_.back());
                menus_.pop_back();
            }
            else
            {
                curr_tag_ = TagType::Boxnope;
            }

            break;

        case TagType::Action:
            addMenuAction(app_, menus_.back(), curr_action_);
            curr_tag_ = TagType::Menu;
            break;

        default:
            break;
    }
}

bool validateMenu(const QByteArray& data)
{
    QXmlSchema schema;
    schema.load(QUrl::fromLocalFile(QSL(":/boxnope_menu.xsd")));

    if (!schema.isValid())
        return false;

    QXmlSchemaValidator validator(schema);
    return validator.validate(data);
}

}

QMenu* createMenuFromFile(QApplication* app, QWidget* parent, QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    QByteArray data(file.readAll());

    if (!validateMenu(data))
        return nullptr;

    MenuParser parser(app, parent, data);
    return parser.parse();
}
