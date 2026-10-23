
#include "room.hpp"

Room::Room(int key, int creatorId)
    : key(key), creatorId(creatorId)
{
    members.insert(creatorId);
}

int Room::getKey() const
{
    return key;
}

int Room::getCreatorId() const
{
    return creatorId;
}

bool Room::addMember(int userId)
{
    return members.insert(userId).second;
}

bool Room::removeMember(int userId)
{
    return members.erase(userId) > 0;
}

bool Room::hasMember(int userId) const
{
    return members.find(userId) != members.end();
}

bool Room::isEmpty() const
{
    return members.empty();
}

size_t Room::memberCount() const
{
    return members.size();
}
