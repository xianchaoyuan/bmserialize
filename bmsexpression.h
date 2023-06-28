#ifndef BMSEXPRESSION_H
#define BMSEXPRESSION_H

#include <QtCore>

class BmSExpression;

// 将对象序列化为BmSExpression
template<class T>
BmSExpression serialize(const T &obj);

// 将BmSExpression反序列化为对象
template<class T>
T deserialize(const BmSExpression &sexpress);

/**
 * @brief 序列化表达式
 */
class BmSExpression
{
public:
    enum class Type {
        List,       // 具有标记名和任意数量的子项
        Token,      // 不带引号的值（例如“-12.34”）
        String,     // 带双引号的值（例如“Foo！”）
        LineBreak,  // 列表中的手动换行
    };

    BmSExpression();
    BmSExpression(Type type, const QString &value);
    BmSExpression(const BmSExpression &other);
    ~BmSExpression() = default;

    BmSExpression &operator=(const BmSExpression &rhs) = default;

    bool isList() const { return m_type == Type::List; }
    bool isToken() const { return m_type == Type::Token; }
    bool isString() const { return m_type == Type::String; }
    bool isLineBreak() const { return m_type == Type::LineBreak; }

    Type getType() const { return m_type; }
    const QString &getName() const;
    const QString &getValue() const;
    const QList<BmSExpression> &getChildren() { return m_children; }
    QList<BmSExpression> getChildren(const Type &type) const;
    QList<BmSExpression> getChildren(const QString &name) const;
    //! 通过路径获取一个子节点
    const BmSExpression &getChild(const QString &path) const;
    //! 尝试通过路径获取一个子节点
    const BmSExpression *tryGetChild(const QString &path) const;
    BmSExpression &appendList(const QString &name);
    BmSExpression &appendChild(const BmSExpression &child);

    template <class T>
    BmSExpression &appendChild(const T &obj) {
        appendChild(serialize(obj));
        return *this;;
    }
    template <class T>
    BmSExpression &appendChild(const QString &child, const T &obj) {
        return appendList(child).appendChild(obj);
    }

    //! 确保换行
    void ensureLineBreak();
    void ensureLineBreakIfMultiLine();

    static BmSExpression createList(const QString &name);
    static BmSExpression createToken(const QString &token);
    static BmSExpression createString(const QString &string);
    static BmSExpression createLineBreak();

    static void skipWhitespaceAndComments(const QString &content, int &index, bool skipNewline = false);
    static BmSExpression parse(const QByteArray &content);

    QByteArray toByteArray() const;

private:
    static BmSExpression parse(const QString &content, int &index);
    static BmSExpression parseList(const QString &content, int &index);
    static QString parseToken(const QString &content, int &index);
    static QString parseString(const QString &content, int &index);

    static bool isValidToken(const QString &token);
    static bool isValidTokenChar(const QChar &c);

    bool isMultiLine() const;
    static bool skipLineBreaks(const QList<BmSExpression> &children, int &index);

    QString toString(int indent) const;

private:
    Type m_type;
    QString m_value;
    QList<BmSExpression> m_children;
};

// 基本类型序列化
template <>
inline BmSExpression serialize(const bool &obj) {
    return BmSExpression::createToken(obj ? "true" : "false");
}

template <>
inline BmSExpression serialize(const int &obj) {
    return BmSExpression::createToken(QString::number(obj));
}

template <>
inline BmSExpression serialize(const QString &obj) {
    return BmSExpression::createString(obj);
}

template <>
inline BmSExpression serialize(const QUuid &obj) {
    return BmSExpression::createToken(obj.toString());
}

// 基本类型反序列化
template <>
inline bool deserialize(const BmSExpression &sexpr) {
    if (sexpr.getValue() == "true")
        return true;
    else if (sexpr.getValue() == "false")
        return false;
    else
        Q_ASSERT(false);
    return false;
}

template <>
inline int deserialize(const BmSExpression &sexpr) {
    bool ok = false;
    int value = sexpr.getValue().toInt(&ok);
    if (ok)
        return value;
    else
        Q_ASSERT(false);
    return -1;
}

template <>
inline QString deserialize(const BmSExpression &sexpr) {
    return sexpr.getValue();
}

template <>
inline QUuid deserialize(const BmSExpression &sexpr) {
    return QUuid::fromString(sexpr.getValue());
}

#endif // BMSEXPRESSION_H
