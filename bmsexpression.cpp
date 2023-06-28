#include "bmsexpression.h"

BmSExpression::BmSExpression()
    : m_type(Type::String)
{}

BmSExpression::BmSExpression(Type type, const QString &value)
    : m_type(type)
    , m_value(value)
{}

BmSExpression::BmSExpression(const BmSExpression &other)
    : m_type(other.m_type)
    , m_value(other.m_value)
    , m_children(other.m_children)
{}

const QString &BmSExpression::getName() const
{
    if (!isList()) {
        Q_ASSERT(false);
    }
    return m_value;
}

const QString &BmSExpression::getValue() const
{
    if (!isToken() && !isString()) {
        Q_ASSERT(false);
    }
    return m_value;
}

QList<BmSExpression> BmSExpression::getChildren(const Type &type) const
{
    QList<BmSExpression> children;
    for (const BmSExpression &child : m_children) {
        if (child.getType() == type) {
            children.append(child);
        }
    }
    return children;
}

QList<BmSExpression> BmSExpression::getChildren(const QString &name) const
{
    QList<BmSExpression> children;
    for (const BmSExpression &child : m_children) {
        if (child.isList() && child.getName() == name) {
            children.append(child);
        }
    }
    return children;
}

const BmSExpression &BmSExpression::getChild(const QString &path) const
{
    const BmSExpression *child = tryGetChild(path);
    if (!child) {
        Q_ASSERT(false);
    }
    return *child;
}

const BmSExpression *BmSExpression::tryGetChild(const QString &path) const
{
    const BmSExpression *child = this;
    for (const QString &name : path.split('/')) {
        if (name.startsWith('@')) {
            bool valid = false;
            int index = name.midRef(1).toInt(&valid);
            if ((valid) && (index >= 0) && skipLineBreaks(child->m_children, index)) {
                child = &child->m_children.at(index);
            } else {
                return nullptr;
            }
        } else {
            bool found = false;
            for (const BmSExpression &childChild : child->m_children) {
                if (childChild.isList() && (childChild.m_value == name)) {
                    child = &childChild;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return nullptr;
            }
        }
    }
    return child;
}

BmSExpression &BmSExpression::appendList(const QString &name)
{
    return appendChild(createList(name));
}

BmSExpression &BmSExpression::appendChild(const BmSExpression &child)
{
    if (m_type != Type::List) {
        Q_ASSERT(false);
    }
    m_children.append(child);
    return m_children.last();
}

void BmSExpression::ensureLineBreak()
{
    if (m_children.isEmpty() || (!m_children.last().isLineBreak())) {
        m_children.append(createLineBreak());
    }
}

void BmSExpression::ensureLineBreakIfMultiLine()
{
    if (isMultiLine()) {
        ensureLineBreak();
    }
}

BmSExpression BmSExpression::createList(const QString &name)
{
    return BmSExpression(Type::List, name);
}

BmSExpression BmSExpression::createToken(const QString &token)
{
    return BmSExpression(Type::Token, token);
}

BmSExpression BmSExpression::createString(const QString &string)
{
    return BmSExpression(Type::String, string);
}

BmSExpression BmSExpression::createLineBreak()
{
    return BmSExpression(Type::LineBreak, QString());
}

void BmSExpression::skipWhitespaceAndComments(const QString &content, int &index, bool skipNewline)
{
    // '\r':空格、回车符 '\t':水平制表符 '\v':垂直制表符 '\f':换页符
    static QSet<QChar> spaces = {' ', '\r', '\t', '\v', '\f'};

    bool isComment = false;
    while (index < content.length()) {
        const QChar &c = content.at(index);
        if (c == ';') { // Lisp 语言的行注释
            isComment = true;
        } else if (c == '\n') {
            isComment = false;
        }

        if (isComment || (skipNewline && (c == '\n')) || spaces.contains(c)) {
            ++index;
        } else {
            break;
        }
    }
}

BmSExpression BmSExpression::parse(const QByteArray &content)
{
    int index = 0;
    QString contentStr = QString::fromUtf8(content);
    skipWhitespaceAndComments(contentStr, index, true);  // 跳过换行符
    if (index >= contentStr.length()) {
        Q_ASSERT(false);
    }

    BmSExpression root = parse(contentStr, index);
    skipWhitespaceAndComments(contentStr, index, true);  // 跳过换行符
    if (index < contentStr.length()) {
        Q_ASSERT(false);
    }
    return root;
}

QByteArray BmSExpression::toByteArray() const
{
    QString str = toString(0);
    if (!str.endsWith('\n')) {
        str += '\n';  // 文件末尾的换行符
    }
    return str.toUtf8();
}

BmSExpression BmSExpression::parse(const QString &content, int &index)
{
    Q_ASSERT(index < content.length());

    QChar c = content.at(index);
    if (c == '\n') {
        ++index;
        skipWhitespaceAndComments(content, index);
        return createLineBreak();
    } else if (c == '(') {
        return parseList(content, index);
    } else if (c == '"') {
        return createString(parseString(content, index));
    } else {
        return createToken(parseToken(content, index));
    }
}

BmSExpression BmSExpression::parseList(const QString &content, int &index)
{
    Q_ASSERT((index < content.length()) && (content.at(index) == '('));

    ++index;  // 去除 '('

    BmSExpression list = createList(parseToken(content, index));
    while (true) {
        if (index >= content.length()) {
            Q_ASSERT(false);
        }

        if (content.at(index) == ')') {
            ++index;
            skipWhitespaceAndComments(content, index);
            break;
        } else {
            list.appendChild(parse(content, index));
        }
    }
    return list;
}

QString BmSExpression::parseToken(const QString &content, int &index)
{
    int oldIndex = index;
    while ((index < content.length()) && (isValidTokenChar(content.at(index)))) {
        ++index;
    }
    QString token = content.mid(oldIndex, index - oldIndex);
    if (token.isEmpty()) {
        Q_ASSERT(false);
    }
    skipWhitespaceAndComments(content, index);
    return token;
}

QString BmSExpression::parseString(const QString &content, int &index)
{
    ++index;  // 去除 '"'

    QString string;
    while (true) {
        if (index >= content.length()) {
            Q_ASSERT(false);
        }

        const QChar &c = content.at(index);
        if (c == '"') {
            ++index;
            skipWhitespaceAndComments(content, index);  // 去除空格和注释
            break;
        } else {
            string += c;
            ++index;
        }
    }
    return string;
}

bool BmSExpression::isValidToken(const QString &token)
{
    if (token.isEmpty()) {
        return false;
    }
    bool ret = std::all_of(token.begin(), token.end(), [](const QChar &c) {
        return isValidTokenChar(c);
    });
    return ret;
}

bool BmSExpression::isValidTokenChar(const QChar &c)
{
    static QSet<QChar> allowedSpecialChars = {'\\', '.', ':', '_', '-', '{', '}'};
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
           ((c >= '0') && (c <= '9')) || allowedSpecialChars.contains(c);
}

bool BmSExpression::isMultiLine() const
{
    if (isLineBreak()) {
        return true;
    } else if (isList()) {
        for (const BmSExpression &child : m_children) {
            if (child.isMultiLine()) {
                return true;
            }
        }
    }
    return false;
}

bool BmSExpression::skipLineBreaks(const QList<BmSExpression> &children, int &index)
{
    for (int i = 0; i < children.count(); ++i) {
        if (children.at(i).isLineBreak()) {
            ++index;
        } else if (i == index) {
            return true;
        }
    }
    return false;
}

QString BmSExpression::toString(int indent) const
{
    if (m_type == Type::List) {
        if (!isValidToken(m_value)) {
            Q_ASSERT(false);
        }

        QString str = '(' + m_value;
        bool lastCharIsSpace = false;
        const int lastIndex = m_children.count() - 1;
        for (int i = 0; i <= lastIndex; ++i) {
            const BmSExpression &child = m_children.at(i);
            if ((!lastCharIsSpace) && (!child.isLineBreak())) {
                str += ' ';
            }
            const bool nextChildIsLineBreak = (i < lastIndex) && m_children.at(i + 1).isLineBreak();
            int currentIndent = (child.isLineBreak() && nextChildIsLineBreak) ? 0 : (indent + 1);
            lastCharIsSpace = child.isLineBreak() && (currentIndent > 0);
            if (lastCharIsSpace && (i == lastIndex)) {
                --currentIndent;
            }
            str += child.toString(currentIndent);
        }
        return str + ')';
    } else if (m_type == Type::Token) {
        if (!isValidToken(m_value)) {
            Q_ASSERT(false);
        }
        return m_value;
    } else if (m_type == Type::String) {
        return '"' + m_value + '"';
    } else if (m_type == Type::LineBreak) {
        return '\n' + QString(' ').repeated(indent);
    } else {
        Q_ASSERT(false);
    }
    return QString{};
}
