
#ifndef TYPES_H
#define TYPES_H
#include <string>
#include <vector>
#include <map>
namespace gateway::common
{
#define DEFINE_VECTOR_FOR_TYPE(t, n)              \
    typedef std::vector<t> n;                     \
    typedef std::vector<t>::iterator n##Iterator; \
    typedef std::vector<t>::const_iterator n##ConstIterator;

#define DEFINE_MAP_FOR_TYPE(t1, t2, n)                         \
    typedef std::map<t1, t2> n;                                \
    typedef std::map<t1, t2>::iterator n##Iterator;            \
    typedef std::map<t1, t2>::const_iterator n##ConstIterator; \
    typedef std::pair<t1, t2> n##Pair;

    DEFINE_VECTOR_FOR_TYPE(std::string, StringVector);
}
#endif // __TYPES_H__