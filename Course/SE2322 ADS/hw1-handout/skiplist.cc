#include "skiplist.h"
#include <optional>
#include <ctime>

namespace skiplist
{

    int skiplist_type::randomLevel()
    {
        int level = 1;
        while (static_cast<double>(std::rand()) / RAND_MAX < p && level <= maxLevel)
        {
            level++;
        }
        return level;
    }

    skiplist_type::skiplist_type(double p) : 
    p(p), maxLevel(16), currentLevel(1)
    {
        header = new skiplistNode(maxLevel + 1, 0, "");
    }

    skiplist_type::~skiplist_type()
    {
        skiplistNode *pt = header;
        while (pt)
        {
            skiplistNode *qt = pt->forward[1];
            delete pt;
            pt = qt;
        }
    }

    void skiplist_type::put(key_type key, const value_type &val)
    {
        std::vector<skiplistNode *> update(maxLevel + 1, nullptr);
        skiplistNode *x = header;
        for (int i = currentLevel; i >= 1; i--)
        {
            while (x->forward[i] != nullptr && x->forward[i]->key < key)
                x = x->forward[i];
            update[i] = x;
        }
        x = x->forward[1];
        if (x != nullptr && x->key == key)
            x->value = val;
        else
        {
            int level = randomLevel();
            if (level > currentLevel)
            {
                for (int i = currentLevel + 1; i <= level; i++)
                    update[i] = header;
                currentLevel = std::min(level, maxLevel);
            }
            x = new skiplistNode(level, key, val);
            for (int i = 1; i <= level; i++)
            {
                x->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = x;
            }
        }
    }

    std::string skiplist_type::get(key_type key) const
    {
        skiplistNode *x = header;
        for (int i = currentLevel; i >= 1; i--)
        {
            while (x->forward[i] != nullptr && x->forward[i]->key < key)
                x = x->forward[i];
        }
        x = x->forward[1];
        if (x != nullptr && x->key == key)
            return x->value;
        else
            return "";
    }

    int skiplist_type::query_distance(key_type key) const
    {
        int dis = 1;
        skiplistNode *x = header;
        for (int i = currentLevel; i >= 1; i--)
        {
            while (x->forward[i] != nullptr && x->forward[i]->key < key)
                x = x->forward[i], dis++;
            if(x->forward[i] != nullptr && x->forward[i]->key == key){
                dis++;
                return dis;
            }
            dis++;
        }
        x = x->forward[1];
        return dis;
    }

} // namespace skiplist