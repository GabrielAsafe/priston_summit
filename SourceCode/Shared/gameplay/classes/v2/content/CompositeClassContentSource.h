#pragma once

#include <string>
#include <vector>

#include "gameplay/classes/v2/content/IClassContentSource.h"

namespace gameplay
{
namespace classes
{
// Small composition root for class content. It deliberately owns no source;
// callers retain source lifetime and choose the ordering explicitly.
class CompositeClassContentSource : public IClassContentSource
{
public:
    CompositeClassContentSource& Add(const IClassContentSource& source)
    {
        sources_.push_back(&source);
        return *this;
    }

    CompositeClassContentSource& Add(const IClassContentSource&&) = delete;

    virtual bool Load(std::vector<PlayableClassDefinition>& out,
                      std::string* error = 0) const
    {
        std::vector<PlayableClassDefinition> staged;
        for (std::vector<const IClassContentSource*>::const_iterator it = sources_.begin();
             it != sources_.end(); ++it)
        {
            std::vector<PlayableClassDefinition> chunk;
            std::string sourceError;
            if (!(*it) || !(*it)->Load(chunk, &sourceError))
            {
                if (error)
                    *error = sourceError.empty() ? "class content source failed" : sourceError;
                return false;
            }
            staged.insert(staged.end(), chunk.begin(), chunk.end());
        }

        out.swap(staged);
        if (error) error->clear();
        return true;
    }

    std::size_t SourceCount() const { return sources_.size(); }

private:
    std::vector<const IClassContentSource*> sources_;
};
}
}
