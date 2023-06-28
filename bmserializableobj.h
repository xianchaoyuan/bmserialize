#ifndef BMSERIALIZABLEOBJ_H
#define BMSERIALIZABLEOBJ_H

#include <QtCore>

#include "bmsexpression.h"

/**
 * @brief 可序列化对象
 */
class BmSerializableObj
{
public:
    BmSerializableObj() {}
    virtual ~BmSerializableObj() {}

    //! 将对象序列化为新的S表达式节点
    BmSExpression serializeToDomElement(const QString &name) const {
        BmSExpression root = BmSExpression::createList(name);
        serialize(root);
        return root;
    }

    //! 将对象序列化到现有的S表达式节点中
    virtual void serialize(BmSExpression &root) const = 0;

    //! 序列化对象容器
    template <class T>
    static void serializeObjectContainer(BmSExpression &root,
                                         const T &container,
                                         const QString &itemName) {
        for (const auto &obj : container) {
            root.ensureLineBreak();
            root.appendChild(obj.serializeToDomElement(itemName));
        }
        root.ensureLineBreakIfMultiLine();
    }
    //! 序列化指针容器
    template <class T>
    static void serializePointerContainer(BmSExpression &root,
                                         const T &container,
                                         const QString &itemName) {
        for (const auto &pointer : container) {
            root.ensureLineBreak();
            root.appendChild(pointer->serializeToDomElement(itemName));
        }
        root.ensureLineBreakIfMultiLine();
    }
    //! 序列化指针容器，并按uuid排序
    template <class T>
    static void serializePointerContainerUuidSorted(BmSExpression &root,
                                                    const T &container,
                                                    const QString &itemName) {
        T copy = container;
        std::sort(copy.begin(), copy.end(), [](const class T::value_type &a, const class T::value_type &b){
            return a->getUuid() < b->getUuid();
        });
        serializeObjectContainer(root, copy, itemName);
    }
};

//! 确保BmSerializableObj类不包含任何数据
static_assert(sizeof(BmSerializableObj) == sizeof(void*),
              "BmSerializableObj must not contain any data!");

#endif // BMSERIALIZABLEOBJ_H
