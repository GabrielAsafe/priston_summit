#pragma once
// SUMMIT 392A — Crafting page in CharacterWindow (same V2 shell as Covenants).
// Global recipe/icon assets; displayed bag items come from the local legacy inventory gateway.
#include <algorithm>
#include <chrono> // SUMMIT404A_AUTO_SNAPSHOT
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream> // recipes_392.tsv, names_392.tsv: global catalog only
#include <functional>
#include <limits>
#include <map>
#include <utility>
#include <sstream>
#include <string>
#include <vector>

#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/domain/IInventoryGateway.h" // SUMMIT410A client backpack projection
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/pages/CraftingIconCache393.h" // SUMMIT393_ICONS_HEADER
#include "systems/ui/v2/pages/CraftUiRegistry426.generated.h" // SUMMIT426A_UNIVERSAL_UI

namespace ui::v2 {
class CraftingPage392 final : public Container {
public:
    using CommandSender = std::function<bool(const std::string&)>;
    using CharacterName = std::function<std::string()>;
    explicit CraftingPage392(UIId id = InvalidUIId) : Container(id) {
        static const char* tabs[] = {"Miner", "Lumberjack", "Fisher", "Hunter", "Artisan", "Blacksmith", "Arcanist"};
        for (int i = 0; i < 7; ++i) {
            Button& b = AddChild<Button>(39210 + i, tabs[i]);
            b.SetBounds({8 + i * 94, 9, 90, 29});
            b.SetOnClick([this, i](const UIEvent&) { profession_ = i; recipePage_ = 0; selectedRecipe_ = -1; RefreshControls(); });
            professionTabs_[i] = &b;
        }
        Button& all426=AddChild<Button>(39217,"Todos");
        all426.SetBounds({666,9,90,29});
        all426.SetOnClick([this](const UIEvent&) { profession_=7; recipePage_=0; selectedRecipe_=-1; RefreshControls(); });
        professionTabs_[7]=&all426;
        Button& grpPrev426=AddChild<Button>(39256,"< Grupo");
        grpPrev426.SetBounds({212,45,83,27});
        grpPrev426.SetOnClick([this](const UIEvent&) {ChangeGroup426(-1);});
        groupPrev426_=&grpPrev426;
        Button& grpNext426=AddChild<Button>(39257,"Grupo >");
        grpNext426.SetBounds({303,45,89,27});
        grpNext426.SetOnClick([this](const UIEvent&) {ChangeGroup426(1);});
        groupNext426_=&grpNext426;
        for (int i=0; i<8; ++i) {
            Button& b=AddChild<Button>(39230+i, "--");
            b.SetBounds({16, 74+i*34, 377, 29});
            b.SetOnClick([this,i](const UIEvent&) { SelectVisibleRecipe(i); });
            recipeButtons_[i]=&b;
        }
        // SUMMIT427A: explicit keyboard lookup, no chat commands needed.
        Button& find427=AddChild<Button>(39259,"Pesquisar ID");
        find427.SetBounds({149,349,112,29});
        find427.SetOnClick([this](const UIEvent&) {
            searchActive427_=true;searchCode427_.clear();
            status_="Pesquisar ID: escreve o codigo (ex. WA103), Enter confirma, Esc cancela.";
        });
        findButton427_=&find427;
        Button& prev=AddChild<Button>(39250,"< Receitas");
        prev.SetBounds({16, 349, 127, 29});
        prev.SetOnClick([this](const UIEvent&) { if(recipePage_>0)--recipePage_;selectedRecipe_=-1;RefreshControls(); });
        prevRecipes_=&prev;
        Button& next=AddChild<Button>(39251,"Receitas >");
        next.SetBounds({266,349,127,29});
        next.SetOnClick([this](const UIEvent&) { if((recipePage_+1)*8 < FilteredCount())++recipePage_;selectedRecipe_=-1;RefreshControls(); });
        nextRecipes_=&next;
        Button& craft=AddChild<Button>(39252,"Fabricar (servidor)");
        craft.SetBounds({16, 424, 377, 31});
        craft.SetOnClick([this](const UIEvent&) { RequestCraft(); });
        craftButton_=&craft;
        Button& refresh=AddChild<Button>(39253,"Atualizar materiais");
        refresh.SetBounds({410, 423, 192, 31});
        refresh.SetOnClick([this](const UIEvent&) { RefreshBackpack410();RefreshControls();status_="Mochila atualizada localmente."; });
        refreshButton_=&refresh;
        Button& prevInv=AddChild<Button>(39254,"< Itens");
        prevInv.SetBounds({416, 374, 116, 29});
        prevInv.SetOnClick([this](const UIEvent&) { if(inventoryPage_>0)--inventoryPage_;RefreshControls(); });
        prevInventory_=&prevInv;
        Button& nextInv=AddChild<Button>(39255,"Itens >");
        nextInv.SetBounds({638, 374, 113, 29});
        nextInv.SetOnClick([this](const UIEvent&) { if((inventoryPage_+1)*11<backpack410_.size())++inventoryPage_;RefreshControls(); });
        nextInventory_=&nextInv;
        RefreshControls();
    }
    void SetSender(CommandSender value) { sender_=std::move(value); }
    void SetInventoryGateway(const IInventoryGateway* gateway) { bagGateway410_=gateway;RefreshBackpack410(); }
    void SetCharacterName(CharacterName value) { characterName_=std::move(value); }
    // SUMMIT410A: the only internal network message required for this page is
    // the single receipt of a user-initiated craft. Never stream balances through chat.
    void AcceptLive407(const std::string& frame) {
        if(frame.compare(0,6,"@S407|")!=0 || frame.size()>120)return;
        const std::size_t r=frame.find("|R|");
        if(r==std::string::npos || frame.find('|',r+3)!=std::string::npos)return;
        const std::string request=frame.substr(6,r-6);
        if(request.empty() || request!=pendingCraft407_)return;
        const std::string outcome=frame.substr(r+3);
        if(outcome=="OK")status_="Fabricacao confirmada. Mochila fisica inalterada por receitas SQLite.";
        else if(outcome=="ERR")status_="Fabricacao recusada. Consulta o motivo no chat.";
        else return;
        pendingCraft407_.clear();
        RefreshBackpack410();
        RefreshControls();
    }
    // SUMMIT427A: capture text only while the user explicitly opened the code lookup.
    UIEventResult OnEvent(const UIEvent& event) override {
        if(searchActive427_) {
            if(event.type==UIEventType::TextInput) {
                for(unsigned char ch:event.textUtf8) {
                    if(searchCode427_.size()>=16)break;
                    if((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9'))
                        searchCode427_.push_back(static_cast<char>(std::toupper(ch)));
                }
                status_="Pesquisar: "+searchCode427_+" | Enter: localizar | Esc: cancelar";
                return UIEventResult::Handled;
            }
            if(event.type==UIEventType::KeyDown) {
                if(event.key==13) {searchActive427_=false;LocateCode427(searchCode427_);return UIEventResult::Handled;}
                if(event.key==27) {searchActive427_=false;status_="Pesquisa cancelada.";return UIEventResult::Handled;}
                if(event.key==8) {
                    if(!searchCode427_.empty())searchCode427_.pop_back();
                    status_="Pesquisar: "+searchCode427_+" | Enter: localizar";
                    return UIEventResult::Handled;
                }
            }
        }
        return Container::OnEvent(event);
    }
    void ForceRefresh() {
        // Only global recipe/name catalogs and the already loaded client backpack.
        recipes_.clear();names_.clear();loaded_=false;
        recipePage_=0;inventoryPage_=0;selectedRecipe_=-1;
        LoadNames();LoadRecipes();groups426_.clear();LoadUniversal426();loaded_=!recipes_.empty();RefreshBackpack410();
        status_="Mochila local. Receitas [MAT] ainda usam SQLite no servidor.";
        RefreshControls();
    }

protected:
    void OnRender(UIRenderContext& ctx) override {
        // SUMMIT411A: release the click cooldown on the local UI clock.
        // Previously RefreshControls() was called only on click/tab changes,
        // leaving Fabricar disabled forever when a receipt did not arrive.
        if(craftCooldownActive411_ && std::chrono::steady_clock::now()>=nextCraftRequest406_) {
            craftCooldownActive411_=false;
            RefreshControls();
        }
        RefreshBackpack410(); // local inventory projection: no chat, SQLite, watcher or polling network
        auto& r=ctx.Renderer();
        const Rect origin=GlobalBounds();
        auto pos=[&](int x,int y,int w,int h){return Rect{origin.x+x,origin.y+y,w,h};};
        r.DrawRect(pos(8,44,390,409),UITheme::Surface);
        r.DrawRect(pos(408,44,352,364),UITheme::Surface);
        Draw(r,pos(18,50,185,22),"Receitas ("+std::to_string(FilteredCount())+")"+
             (profession_==7&&groupIndex426_>0?" "+groups426_[groupIndex426_]:""),UITheme::Accent,true);
        Draw(r,pos(418,50,330,22),"Itens da mochila A+B (leitura)",UITheme::Accent,true);
        if (!loaded_) Draw(r,pos(18,385,380,25),"Catalogo ausente: exportar Content.",UITheme::Danger);
        else if (FilteredCount()==0) Draw(r,pos(18,90,366,32),"Sem receitas nesta profissao.",UITheme::TextMuted);
        else DrawSelected(r,pos);
        DrawInventory(r,pos);
        Draw(r,pos(13,456,735,19),status_,UITheme::TextMuted);
    }
private:
    struct Ingredient {std::string key;int quantity=0;};
    struct Recipe {std::string id, profession, output;int level=0, amount=0;bool physical=false,enabled=false;std::vector<Ingredient> inputs; bool universal426=false; std::uint32_t native426=0; std::string nativeCode426, summary426;};
    struct BagItem410 {std::uint32_t code=0;std::string name;int amount=1;TextureHandle texture{};int sourceBag=1;};
    std::vector<Recipe> recipes_;
    const IInventoryGateway* bagGateway410_=nullptr;
    std::vector<BagItem410> backpack410_;
    bool bagValid410_=false;
    std::string bagPlayer410_;
    CraftingIconCache393 iconCache_; // SUMMIT393_CACHE
    std::map<std::string,std::string> names_;
    std::array<Button*,8> professionTabs_{};
    Button *groupPrev426_=nullptr,*groupNext426_=nullptr;
    Button *findButton427_=nullptr;
    bool searchActive427_=false;
    std::string searchCode427_;
    int activeBag427_=1;
    std::vector<std::string> groups426_;
    std::size_t groupIndex426_=0;
    std::array<Button*,8> recipeButtons_{};
    Button *prevRecipes_=nullptr,*nextRecipes_=nullptr,*craftButton_=nullptr;
    Button *refreshButton_=nullptr,*prevInventory_=nullptr,*nextInventory_=nullptr;
    CommandSender sender_;
    CharacterName characterName_;
    std::string status_="Mochila local. As receitas [MAT] ainda usam SQLite no servidor.";
    int profession_=4,selectedRecipe_=-1;
    std::size_t recipePage_=0, inventoryPage_=0;
    bool loaded_=false;
    std::chrono::steady_clock::time_point nextCraftRequest406_{};
    bool craftCooldownActive411_=false; // local UI control, no network traffic
    std::string pendingCraft407_; // only UI receipt of explicit fabrication

    static const std::array<std::string,7>& Professions() {
        static const std::array<std::string,7> p={"Miner","Lumberjack","Fisher","Hunter","Artisan","Blacksmith","Arcanist"};return p;
    }
    static std::vector<std::string> Columns(const std::string& s) {
        std::vector<std::string> out;std::size_t start=0;
        for(std::size_t i=0;i<=s.size();++i)if(i==s.size()||s[i]=='\t'){out.push_back(s.substr(start,i-start));start=i+1;}
        return out;
    }
    static bool Number(const std::string& s,int& value) {
        if(s.empty()||s.size()>9)return false;
        for(char ch:s)if(ch<'0'||ch>'9')return false;
        const long long x=std::strtoll(s.c_str(),nullptr,10);
        if (x>100000000) return false;
        value=static_cast<int>(x);
        return true;
    }
    static bool ValidKey(const std::string& s) {
        if(s.empty()||s.size()>90)return false;
        for(char c:s)if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='.'||c=='-'))return false;
        return true;
    }
    static std::string Humanize(std::string s) {
        bool up=true;
        for(char& c:s) {
            if(c=='_'||c=='.'){c=' ';up=true;}
            else if(up){c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));up=false;}
        }
        return s;
    }
    std::string Name(const std::string& s) const {
        const auto it=names_.find(s);return it==names_.end()?Humanize(s):it->second;
    }
    void Draw(IUIRenderer& r,const Rect& b,const std::string& t,Color color,bool bold=false) const {
        r.DrawTextUtf8(FontHandle{},t,b,TextStyle{color,bold,TextAlign::Left});
    }
    template <class F> void DrawSelected(IUIRenderer& r,F pos) {
        if (selectedRecipe_<0||static_cast<std::size_t>(selectedRecipe_)>=recipes_.size()){
            Draw(r,pos(18,386,370,23),"Seleciona uma receita para ver ingredientes.",UITheme::TextMuted);return;
        }
        const Recipe& recipe=recipes_[static_cast<std::size_t>(selectedRecipe_)];
        std::string summary="Nivel "+std::to_string(recipe.level)+" | "+Name(recipe.output)+" ["+
            (recipe.nativeCode426.empty()?recipe.output:recipe.nativeCode426)+"] x"+std::to_string(recipe.amount);
        Draw(r,pos(19,384,370,22),summary,UITheme::Text);
        // SUMMIT426A_ICON: avoid resource-key/dummy lookup. Use the authenticated
        // local backpack's actual texture only when the native ID matches.
        bool icon426=false;
        for(const BagItem410& item:backpack410_)
            if(recipe.native426 && item.code==recipe.native426 && item.texture.IsValid()) {
                r.DrawImage(item.texture,pos(364,381,26,26));icon426=true;break;
            }
        if(!icon426 && recipe.native426) Draw(r,pos(312,381,80,20),"[ID nativo]",UITheme::TextMuted);
        std::string inputs;
        for(const Ingredient& ing:recipe.inputs){
            if(!inputs.empty())inputs+=" + ";
            inputs+=Name(ing.key)+" x"+std::to_string(ing.quantity);
        }
        if(recipe.universal426) inputs=recipe.summary426;
        if(inputs.size()>68)inputs=inputs.substr(0,65)+"...";
        Draw(r,pos(19,402,375,22),inputs,UITheme::TextMuted); // requirements validated server-side
    }
    template <class F> void DrawInventory(IUIRenderer& r,F pos) {
        if(!bagValid410_) {
            Draw(r,pos(418,85,330,43),"Inventario da personagem indisponivel.",UITheme::Warning);
            return;
        }
        Draw(r,pos(418,74,328,23),"A+B locais; pagina ativa "+std::string(activeBag427_==1?"A":"B"),UITheme::Positive);
        if(backpack410_.empty())Draw(r,pos(418,123,330,24),"Mochilas A+B vazias.",UITheme::TextMuted);
        const std::size_t start=inventoryPage_*11;
        for(std::size_t i=0;i<11&&start+i<backpack410_.size();++i){
            const BagItem410& item=backpack410_[start+i];
            const int yy=116+static_cast<int>(i)*23;
            if(i%2==0)r.DrawRect(pos(417,yy-1,330,22),UITheme::SurfaceRaised);
            r.DrawRect(pos(420,yy+2,18,17),UITheme::AccentMuted);
            if(item.texture.IsValid())r.DrawImage(item.texture,pos(418,yy-3,24,24));
            const std::string text=item.name.empty()?"Item "+std::to_string(item.code):item.name;
            const std::string tagged=std::string(item.sourceBag==1?"[A] ":"[B] ")+text;
            Draw(r,pos(446,yy,222,22),tagged.size()>28?tagged.substr(0,25)+"...":tagged,UITheme::Text);
            Draw(r,pos(676,yy,68,22),"x"+std::to_string(item.amount),UITheme::Accent,true);
        }
        Draw(r,pos(546,377,85,21),std::to_string(inventoryPage_+1)+" / "+
            std::to_string(std::max<std::size_t>(1,(backpack410_.size()+10)/11)),UITheme::TextMuted);
        Draw(r,pos(610,431,146,22),"Sem rede/SQLite",UITheme::TextMuted);
    }
    void RefreshBackpack410() {
        // This reads the *same* local inventory adapter as the Inventory tab.
        // It does not grant an item and is never an authority for consumption.
        const std::string player=characterName_?characterName_():std::string();
        if(player!=bagPlayer410_){
            bagPlayer410_=player;backpack410_.clear();inventoryPage_=0;
            pendingCraft407_.clear();
        }
        if(player.empty() || !bagGateway410_){backpack410_.clear();bagValid410_=false;return;}
        const InventorySnapshot snapshot=bagGateway410_->GetSnapshot();
        activeBag427_=snapshot.activeBagPage;
        bagValid410_=snapshot.valid;
        backpack410_.clear();
        if(!snapshot.valid)return;
        for(const InventoryItemSnapshot& item:snapshot.items){
            if(item.equipped || item.questItem || item.code==0)continue;
            if(backpack410_.size()>=160)break;
            backpack410_.push_back(BagItem410{item.code,CraftDisplayName427B(item.code,item.name),
                item.potionCount>0?item.potionCount:1,item.texture,activeBag427_});
        }
        // SUMMIT427A: inactive A/B bag is already resident in InvenItemTemp.
        // Never change pages or touch mouse/drag state inside a read-only view.
        const auto inactive427=bagGateway410_->GetInactiveBagForCrafting427();
        for(const InventoryItemSnapshot& item:inactive427){
            if(item.equipped || item.questItem || item.code==0)continue;
            if(backpack410_.size()>=160)break;
            backpack410_.push_back(BagItem410{item.code,CraftDisplayName427B(item.code,item.name),
                item.potionCount>0?item.potionCount:1,item.texture,activeBag427_==1?2:1});
        }
        if(inventoryPage_*11>=backpack410_.size())inventoryPage_=0;
    }
    // SUMMIT427B: display-only correction. Never writes inventory state.
    // The legacy item name can be stale or repeated across unrelated native IDs.
    // Use the compiled 426A native-code registry for a trusted CODE->name lookup.
    static std::string CraftDisplayName427B(std::uint32_t native, const std::string& legacy) {
        struct Entry427B { std::string code, name; };
        static const std::map<std::uint32_t,Entry427B> catalog = [] {
            std::map<std::uint32_t,Entry427B> out;
            std::map<std::uint32_t,bool> ambiguous;
            for(std::size_t i=0;i<summit_craft_ui_426::kItemCount;++i) {
                const auto& row=summit_craft_ui_426::kItems[i];
                if(!row.both || row.native==0u) continue;
                const std::string code=row.code?row.code:"";
                const std::string name=row.name?row.name:"";
                if(code.empty())continue;
                const auto found=out.find(row.native);
                if(found!=out.end() && found->second.code!=code) {
                    ambiguous[row.native]=true;
                    continue;
                }
                if(found==out.end())out.emplace(row.native,Entry427B{code,name});
            }
            for(const auto& collision:ambiguous)out.erase(collision.first);
            return out;
        }();
        const auto found=catalog.find(native);
        if(found==catalog.end()) {
            // No verified native association: show the numeric identity, not
            // a potentially false legacy name such as 'Fera Azul'.
            return "[ID " + std::to_string(native) + "]";
        }
        const Entry427B& entry=found->second;
        const bool canonicalName=!entry.name.empty() && entry.name!=entry.code;
        // For CI crafting materials the 419B/426A name is the authority.
        if(entry.code.size()==5 && entry.code.compare(0,2,"CI")==0) {
            return "["+entry.code+"]"+(canonicalName?" "+entry.name:"");
        }
        // Other entries retain an unambiguous CODE and prefer canonical name;
        // a legacy fallback remains identifiable and is never trusted for debit.
        if(canonicalName) return "["+entry.code+"] "+entry.name;
        if(!legacy.empty()) return "["+entry.code+"] "+legacy;
        return "["+entry.code+"]";
    }
    void LoadNames() {
        std::ifstream in("Content/professions_v2/names_392.tsv",std::ios::binary);
        std::string line;
        if(!std::getline(in,line)||line!="SUMMIT392NAMES\t1")return;
        int rows=0;
        while(std::getline(in,line)&&++rows<500){auto cols=Columns(line);
            if(cols.size()==3&&cols[0]=="N"&&ValidKey(cols[1])&&!cols[2].empty()&&cols[2].size()<96)names_.emplace(cols[1],cols[2]);
        }
    }
    void LoadRecipes() {
        std::ifstream in("Content/professions_v2/recipes_392.tsv",std::ios::binary);
        std::string line;
        if(!std::getline(in,line)||line!="SUMMIT392REC\t1")return;
        int rows=0;
        while(std::getline(in,line)&&++rows<250){auto cols=Columns(line);
            if(cols.size()==9&&cols[0]=="R"){
                int level=0,qty=0,phys=0,en=0;
                if(!ValidKey(cols[1])||!ValidKey(cols[3])||!ValidKey(cols[5])||
                   !Number(cols[4],level)||!Number(cols[6],qty)||!Number(cols[7],phys)||!Number(cols[8],en)||qty==0||phys>1||en>1)continue;
                Recipe recipe;recipe.id=cols[1];recipe.profession=cols[2];recipe.level=level;
                recipe.output=cols[5];recipe.amount=qty;recipe.physical=phys!=0;recipe.enabled=en!=0;
                recipes_.push_back(recipe);
            }else if(cols.size()==4&&cols[0]=="I"&&!recipes_.empty()){
                int qty=0;
                if(cols[1]!=recipes_.back().id||!ValidKey(cols[2])||!Number(cols[3],qty)||qty==0||qty>10000)continue;
                recipes_.back().inputs.push_back(Ingredient{cols[2],qty});
            }
        }
        loaded_=!recipes_.empty();
    }
    // SUMMIT426A: catalog projection generated from 424A; all entries are
    // intentionally disabled for DAT crafting, but queryable by clicking.
    void LoadUniversal426() {
        for(std::size_t i=0;i<summit_craft_ui_426::kItemCount;++i) {
            const auto& row=summit_craft_ui_426::kItems[i];
            Recipe recipe;
            recipe.id=std::string("craft.item.")+row.code;
            recipe.profession="Todos";recipe.output=row.code;
            recipe.amount=row.amount;recipe.physical=true;recipe.enabled=false;
            recipe.universal426=true;recipe.native426=row.native;
            recipe.nativeCode426=row.code;
            recipe.summary426=row.ingredients;
            recipes_.push_back(std::move(recipe));
            names_[row.code]=row.name;
            const std::string prefix=std::string(row.code).substr(0,2);
            if(std::find(groups426_.begin(),groups426_.end(),prefix)==groups426_.end()) groups426_.push_back(prefix);
        }
        std::sort(groups426_.begin(),groups426_.end());
        groups426_.insert(groups426_.begin(),std::string());
        groupIndex426_=0;
        // Historic 57 recipes retain their original SQL identifiers. Only
        // their *icon preview* is remapped using the 424A legacy drafts.
        for(Recipe& r:recipes_) if(!r.universal426) {
            for(std::size_t i=0;i<summit_craft_ui_426::kLegacyCount;++i) {
                const auto& l=summit_craft_ui_426::kLegacy[i];
                if(r.id==l.id) {r.native426=l.native;r.nativeCode426=l.code;break;}
            }
        }
    }
    void LocateCode427(std::string code) {
        if(code.empty()){status_="Escreve um codigo para pesquisar.";return;}
        for(char& ch:code)ch=static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        auto it=std::find_if(groups426_.begin(),groups426_.end(),[&](const std::string& group) {
            return group==code.substr(0,2);
        });
        if(it==groups426_.end()){status_="Codigo "+code+" nao existe no catalogo 424A.";return;}
        profession_=7;
        groupIndex426_=static_cast<std::size_t>(it-groups426_.begin());
        const auto filtered=Filtered();
        for(std::size_t index=0;index<filtered.size();++index) {
            const Recipe& r=recipes_[static_cast<std::size_t>(filtered[index])];
            if(r.universal426&&r.nativeCode426==code) {
                recipePage_=index/8;
                selectedRecipe_=filtered[index];
                status_="Selecionado "+code+": consultar pelo botao (DAT ainda bloqueado).";
                RefreshControls();return;
            }
        }
        status_="Codigo "+code+" nao existe neste catalogo.";
        RefreshControls();
    }
    void ChangeGroup426(int delta) {
        if(profession_!=7 || groups426_.empty())return;
        if(delta<0 && groupIndex426_>0)--groupIndex426_;
        if(delta>0 && groupIndex426_+1<groups426_.size())++groupIndex426_;
        recipePage_=0;selectedRecipe_=-1;RefreshControls();
    }
    // SUMMIT410A: opening the Crafting page never sends network packets.
    // 409A: materials_392.tsv was tied to a single character and is never
    // consulted. Names, recipes and icons are global catalog assets.
    std::vector<int> Filtered() const {
        std::vector<int> idx;
        const std::string& name=Professions()[static_cast<std::size_t>(profession_==7?4:profession_)];
        for(std::size_t i=0;i<recipes_.size();++i)if((profession_==7&&recipes_[i].universal426&&
           (groupIndex426_==0||recipes_[i].output.compare(0,2,groups426_[groupIndex426_])==0))||
           (profession_!=7&&recipes_[i].profession==name))idx.push_back(static_cast<int>(i));
        return idx;
    }
    std::size_t FilteredCount()const {
        std::size_t n=0;const auto& prof=Professions()[static_cast<std::size_t>(profession_==7?4:profession_)];
        for (const Recipe& r:recipes_) if ((profession_==7&&r.universal426&&
            (groupIndex426_==0||r.output.compare(0,2,groups426_[groupIndex426_])==0))||
            (profession_!=7&&r.profession==prof)) ++n;
        return n;
    }
    void SelectVisibleRecipe(int i) {
        auto ids=Filtered();const std::size_t index=recipePage_*8+static_cast<std::size_t>(i);
        if(index>=ids.size())return;
        selectedRecipe_=ids[index];
        const Recipe& chosen421=recipes_[static_cast<std::size_t>(selectedRecipe_)];
        if(chosen421.universal426)
            status_="Receita JSON 424A: consulta pelo botao; fabrico DAT ainda bloqueado.";
        else if(!chosen421.enabled)
            status_="Receita indisponivel no catalogo; nao sera enviado pedido.";
        else if(chosen421.physical)
            status_="Receita DAT: servidor bloqueia esta operacao ate haver entrega fisica transacional.";
        else
            status_="Receita SQLite: clicar Fabricar envia o pedido ao servidor; resultado no chat.";
        RefreshControls(); // SUMMIT421B_BUTTON
    }
    void RefreshControls() {
        const auto ids=Filtered();
        for(int i=0;i<8;++i) {
            professionTabs_[i]->SetSelected(i==profession_);
        }
        groupPrev426_->SetVisible(profession_==7);
        groupNext426_->SetVisible(profession_==7);
        groupPrev426_->SetEnabled(profession_==7&&groupIndex426_>0);
        groupNext426_->SetEnabled(profession_==7&&groupIndex426_+1<groups426_.size());
        for(int i=0;i<8;++i){const std::size_t id=recipePage_*8+static_cast<std::size_t>(i);
            const bool ok=id<ids.size();recipeButtons_[i]->SetVisible(ok);
            if(ok){const Recipe& r=recipes_[static_cast<std::size_t>(ids[id])];
                recipeButtons_[i]->SetText((r.universal426?"["+r.nativeCode426+"] ":(r.physical?"[DAT] ":"[MAT] "))+Name(r.output)+" x"+std::to_string(r.amount));
                recipeButtons_[i]->SetSelected(ids[id]==selectedRecipe_);
            }
        }
        prevRecipes_->SetEnabled(recipePage_>0);
        nextRecipes_->SetEnabled((recipePage_+1)*8<ids.size());
        prevInventory_->SetEnabled(inventoryPage_>0);
        nextInventory_->SetEnabled((inventoryPage_+1)*11<backpack410_.size());
        bool can=false;
        if(selectedRecipe_>=0&&static_cast<std::size_t>(selectedRecipe_)<recipes_.size()){
            const Recipe& r=recipes_[static_cast<std::size_t>(selectedRecipe_)];
            // SUMMIT398A_SERVER_AUTHORITY: snapshot is read-only/stale, never
            // the authority for enabling a safe request to the server.
            // 421B: allows clicking a physical recipe only to show a local warning, NOT to send a request.
            can=(r.enabled||r.universal426||r.physical)&&std::chrono::steady_clock::now()>=nextCraftRequest406_;
        }
        craftButton_->SetEnabled(can);
        if(selectedRecipe_<0 || static_cast<std::size_t>(selectedRecipe_)>=recipes_.size())
            craftButton_->SetText("Escolhe uma receita");
        else if(recipes_[static_cast<std::size_t>(selectedRecipe_)].universal426)
            craftButton_->SetText("Consultar receita JSON (sem fabricar)");
        else if(recipes_[static_cast<std::size_t>(selectedRecipe_)].physical)
            craftButton_->SetText("Consultar materiais fisicos (sem fabricar)");
        else if(!recipes_[static_cast<std::size_t>(selectedRecipe_)].enabled)
            craftButton_->SetText("Receita indisponivel");
        else
            craftButton_->SetText("Fabricar (servidor SQLite)");
        // 406A: never overwrite a craft result/error while redrawing controls.
        // The authoritative receipt still arrives as server chat text.
    }
    void RequestCraft() {
        if(selectedRecipe_<0||static_cast<std::size_t>(selectedRecipe_)>=recipes_.size())return;
        const Recipe& r=recipes_[static_cast<std::size_t>(selectedRecipe_)];
        if(r.universal426 || r.physical) {
            const auto now426=std::chrono::steady_clock::now();
            if(now426<nextCraftRequest406_) {status_="Consulta recente; tenta novamente.";return;}
            if(!characterName_||characterName_().empty()) {status_="Sem personagem autenticada.";return;}
            const std::string cmd426=r.universal426 ? "/craft plan "+r.nativeCode426 : "/craft check "+r.id;
            if(!sender_||!sender_(cmd426)) {status_="ERRO: consulta nao enviada.";return;}
            nextCraftRequest406_=now426+std::chrono::milliseconds(900);
            craftCooldownActive411_=true;
            status_="Consulta enviada pelo botao. Resposta do servidor no chat; nenhum item gasto.";
            RefreshControls();
            return;
        }
        if(!r.enabled){status_="Receita desativada: nenhum pedido enviado.";return;}
        const auto now406=std::chrono::steady_clock::now();
        if(now406<nextCraftRequest406_){status_="Pedido recente: evitar clique duplicado; confirma no chat.";return;}
        // SUMMIT398A_AUTHENTICATED: login and recipe authorization are checked
        // by the existing server craft handler, never by client snapshot.
        if(!characterName_ || characterName_().empty()){
            status_="Sem personagem autenticada; nao enviado.";return;
        }
        // Never infer success from a local snapshot: await the server receipt.
        // 406A: one bounded replay key per intentional click, scoped to the
        // authenticated player on the server. SQLite migration 050 is needed.
        static std::uint64_t serial406=0;
        const auto ticks406=std::chrono::system_clock::now().time_since_epoch().count();
        const std::string request406="c406_"+
            std::to_string(static_cast<std::uint64_t>(ticks406))+"_"+
            std::to_string(++serial406);
        pendingCraft407_=request406;
        const std::string command="/craft make "+r.id+" "+request406;
        if(!sender_||!sender_(command)){
            pendingCraft407_.clear();
            status_="ERRO: pedido nao enviado; verifica login e ligacao.";return;
        }
        nextCraftRequest406_=now406+std::chrono::milliseconds(3000);
        craftCooldownActive411_=true;
        RefreshControls(); // disables the button for only three seconds
        // The server sends one human-readable result. Never wait for an
        // optional @S407 chat receipt; it is not a reliable private channel.
        status_="Pedido enviado. Resultado (sucesso ou erro) no chat.";
    }
};
} // namespace ui::v2
