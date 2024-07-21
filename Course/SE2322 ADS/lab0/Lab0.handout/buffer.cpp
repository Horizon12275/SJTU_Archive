
// map for multi char frequency
std::map<std::string, int> multiFrequency;
for (int i = 0; i < text.size() - 1; i++)
{
    std::string temp = text.substr(i, 2);
    multiFrequency[temp]++;
}
struct multicmp // compare for multi char
{
    bool operator()(const hftreeNode &a, const hftreeNode &b)
    {
        if (a.weight == b.weight)
            return a.data > b.data;
        return a.weight < b.weight;
    }
};
std::set<hftreeNode, multicmp> multichar;
// get the max 3 frequency
for (const auto &pair : multiFrequency)
{
    hftreeNode newnode(pair.first, pair.second);
    multichar.insert(newnode);
    if (multichar.size() > 3)
    {
        multichar.erase(multichar.begin());
    }
}
// subtract single char frequency from multi char frequency
for (auto pair : multichar)
{
    for (int i = 0; i < pair.data.size(); i++)
    {
        singleFrequency[std::string(1, pair.data[i])] -= pair.weight;
    }
}