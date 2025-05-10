#ifndef CSINGLEMAP_H
#define CSINGLEMAP_H

#include <QMap>

class IRefCounter
{
public:
    IRefCounter(){ refCount=0; }
    int refCount;
};

template<class Key, class T>

class CSingleMap : public QMap<Key,T*>
{
    public:
        static T* addItem(const Key& k)
        {
            T* item;
            if (getInstance()->contains(k))
            {
                item = getInstance()->value(k);
                (static_cast<IRefCounter*>(item))->refCount++;
                //qDebug() << "refcount++" << (static_cast<IRefCounter*>(item))->refCount;
            }
            else
            {
                item = new T;
                (static_cast<IRefCounter*>(item))->refCount=1;
                getInstance()->insert(k,item);
                //qDebug() << "refcount=1" << (static_cast<IRefCounter*>(item))->refCount;
            }
            return item;
        }
        static void removeItem(const Key& k)
        {
            if (getInstance()->contains(k))
            {
                T* item = getInstance()->value(k);
                //qDebug() << "refcount--" << (static_cast<IRefCounter*>(item))->refCount;
                if (--((static_cast<IRefCounter*>(item))->refCount)==0)
                {
                    getInstance()->remove(k);
                    delete item;
                }
            }
        }
    private:
        static CSingleMap* getInstance()
        {
            static CSingleMap    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return &instance;
        }
        CSingleMap() {
        }                   // Constructor? (the {} brackets) are needed here.
        CSingleMap(CSingleMap const&);              // Don't Implement
        void operator=(CSingleMap const&); // Don't implement
};

#endif // CSINGLEMAP_H
