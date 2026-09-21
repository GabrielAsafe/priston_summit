#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gameplay
{
namespace events
{
class GameplayEventHub
{
public:
    typedef std::size_t SubscriptionId;

    GameplayEventHub() : nextId_(1) {}
    GameplayEventHub(const GameplayEventHub&) = delete;
    GameplayEventHub& operator=(const GameplayEventHub&) = delete;

    template <typename Event, typename Handler>
    SubscriptionId Subscribe(Handler&& handler)
    {
        Channel<Event>& channel = GetOrCreateChannel<Event>();
        const SubscriptionId id = nextId_++;
        channel.handlers.push_back(typename Channel<Event>::Entry{
            id,
            std::function<void(const Event&)>(std::forward<Handler>(handler))
        });
        return id;
    }

    template <typename Event>
    bool Unsubscribe(SubscriptionId id)
    {
        Channel<Event>* channel = GetChannel<Event>();
        if (!channel)
            return false;

        typename std::vector<typename Channel<Event>::Entry>::iterator it =
            std::remove_if(channel->handlers.begin(), channel->handlers.end(),
                           [id](const typename Channel<Event>::Entry& entry)
                           {
                               return entry.id == id;
                           });
        if (it == channel->handlers.end())
            return false;
        channel->handlers.erase(it, channel->handlers.end());
        return true;
    }

    template <typename Event>
    void Publish(const Event& event)
    {
        Channel<Event>* channel = GetChannel<Event>();
        if (!channel)
            return;

        // A consumer may subscribe/unsubscribe while handling an event. Copying
        // the small callback list keeps iteration stable and deterministic.
        const std::vector<typename Channel<Event>::Entry> handlers = channel->handlers;
        for (typename std::vector<typename Channel<Event>::Entry>::const_iterator it = handlers.begin();
             it != handlers.end(); ++it)
        {
            if (it->handler)
                it->handler(event);
        }
    }

    template <typename Event>
    std::size_t SubscriberCount() const
    {
        const Channel<Event>* channel = GetChannel<Event>();
        return channel ? channel->handlers.size() : 0;
    }

    void Clear()
    {
        channels_.clear();
        nextId_ = 1;
    }

private:
    struct IChannel
    {
        virtual ~IChannel() {}
    };

    template <typename Event>
    struct Channel : IChannel
    {
        struct Entry
        {
            SubscriptionId id;
            std::function<void(const Event&)> handler;
        };

        std::vector<Entry> handlers;
    };

    template <typename Event>
    Channel<Event>* GetChannel()
    {
        const std::type_index key(typeid(Event));
        typename ChannelMap::iterator it = channels_.find(key);
        if (it == channels_.end())
            return 0;
        return static_cast<Channel<Event>*>(it->second.get());
    }

    template <typename Event>
    const Channel<Event>* GetChannel() const
    {
        const std::type_index key(typeid(Event));
        typename ChannelMap::const_iterator it = channels_.find(key);
        if (it == channels_.end())
            return 0;
        return static_cast<const Channel<Event>*>(it->second.get());
    }

    template <typename Event>
    Channel<Event>& GetOrCreateChannel()
    {
        Channel<Event>* channel = GetChannel<Event>();
        if (channel)
            return *channel;

        std::unique_ptr<Channel<Event> > created(new Channel<Event>());
        Channel<Event>* raw = created.get();
        channels_[std::type_index(typeid(Event))] = std::move(created);
        return *raw;
    }

    typedef std::unordered_map<std::type_index, std::unique_ptr<IChannel> > ChannelMap;

    ChannelMap channels_;
    SubscriptionId nextId_;
};
}
}
