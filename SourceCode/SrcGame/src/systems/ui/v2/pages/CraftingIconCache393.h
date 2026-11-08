#pragma once
// SUMMIT 393A: PNGs already copied by 391A, D3D9 managed textures.
// Purely visual; never changes physical items, DAT, SQLite, or recipes.
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <d3d9.h>
#include <d3dx9.h>
#include "systems/rendering/legacy_3d/smDsx.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui { namespace v2 {
class CraftingIconCache393 {
public:
    ~CraftingIconCache393() { Clear(); }
    CraftingIconCache393(const CraftingIconCache393&) = delete;
    CraftingIconCache393& operator=(const CraftingIconCache393&) = delete;
    CraftingIconCache393() = default;

    void Draw(IUIRenderer& renderer, const std::string& key, const Rect& destination) {
        if (!d3ddev || !ValidKey(key)) return;
        if (device_ != d3ddev) { Clear(); device_ = d3ddev; }
        if (!loadedMap_) ReadMap();
        const auto it=paths_.find(key);
        if (it == paths_.end()) return; // Sheltom/physical icon references stay with legacy items.
        const auto cached=textures_.find(key);
        if (cached != textures_.end()) {
            if (cached->second) renderer.DrawImage(TextureHandle(reinterpret_cast<std::uintptr_t>(cached->second)), destination);
            return;
        }
        // Only allow whitelisted relative paths in image/professions/v2/items/dummy.
        const std::string path=it->second;
        IDirect3DTexture9* texture=nullptr;
        std::ifstream probe(path.c_str(),std::ios::binary);
        if (probe.good()) {
            probe.close();
            const HRESULT hr=D3DXCreateTextureFromFileExA(device_,path.c_str(),
                D3DX_DEFAULT_NONPOW2,D3DX_DEFAULT_NONPOW2,1,0,D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED,D3DX_FILTER_LINEAR,D3DX_FILTER_LINEAR,0,nullptr,nullptr,&texture);
            if (FAILED(hr)) texture=nullptr;
        }
        textures_[key]=texture; // Remember missing file, avoid a failed texture read each frame.
        if (texture) renderer.DrawImage(TextureHandle(reinterpret_cast<std::uintptr_t>(texture)),destination);
    }
private:
    static bool ValidKey(const std::string& key) {
        if (key.empty() || key.size()>90) return false;
        for (unsigned char c:key)
            if (!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='.'||c=='-')) return false;
        return true;
    }
    void ReadMap() {
        loadedMap_=true;
        std::ifstream input("Content/professions_v2/icons_393.tsv",std::ios::binary);
        std::string line;
        if (!std::getline(input,line) || line!="SUMMIT393ICONS\t1") return;
        int lines=0;
        const std::string prefix="image/professions/v2/items/dummy/";
        while (std::getline(input,line) && ++lines<=128) {
            const std::size_t a=line.find('\t');
            if(a==std::string::npos||a!=1||line[0]!='P')continue;
            const std::size_t b=line.find('\t',a+1);
            if(b==std::string::npos)continue;
            const std::string key=line.substr(a+1,b-a-1);
            const std::string path=line.substr(b+1);
            if(!ValidKey(key) || path!=prefix+key+".png")continue;
            paths_[key]=path;
        }
    }
    void Clear() {
        for(auto& entry:textures_) if(entry.second) entry.second->Release();
        textures_.clear();
        device_=nullptr;
    }
    IDirect3DDevice9* device_=nullptr;
    bool loadedMap_=false;
    std::map<std::string,std::string> paths_;
    std::map<std::string,IDirect3DTexture9*> textures_;
};
}} // namespace ui::v2
