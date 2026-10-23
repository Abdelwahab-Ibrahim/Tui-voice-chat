
#pragma once

#include <unordered_set>
#include <cstddef>

class Room
{
public:
    Room(int key, int creatorId);

    int getKey() const;
    int getCreatorId() const;

    bool addMember(int userId);
    bool removeMember(int userId);
    bool hasMember(int userId) const;
    bool isEmpty() const;
    size_t memberCount() const;

private:
    int key;
    int creatorId;
    std::unordered_set<int> members;
};
