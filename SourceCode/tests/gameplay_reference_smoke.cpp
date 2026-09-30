#include <iostream>
#include <vector>
#include "content/v1/gameplay_reference/content/CompiledGameplayReferenceSource.h"
int main(int argc,char**argv){
 if(argc!=2)return 2;
 summit::content::gameplay_reference::CompiledGameplayReferenceSource s(argv[1]);
 if(!s.IsValid()){std::cerr<<s.LastError()<<"\n";return 1;}
 std::vector<summit::content::gameplay_reference::QuestGuideEntry> q;
 if(!s.Quests(q))return 3;
 std::vector<summit::content::gameplay_reference::QuestGuideEntry> sup;
 if(!s.SupportedQuests(sup))return 4;
 summit::content::gameplay_reference::QuestGuideEntry x;
 if(!s.FindByRuntimeId(10030,x))return 5;
 summit::content::gameplay_reference::MixGuideCatalog mixCatalog;
 if(!s.MixCatalog(mixCatalog))return 6;
 std::vector<summit::content::gameplay_reference::MixGuideRecipe> mixes;
 if(!s.MixRecipes(mixes))return 7;
 std::cout<<"quests="<<q.size()<<" supported="<<sup.size()<<" id="<<x.guideId<<" name="<<x.name<<" objectives="<<x.objectives.size()<<" mixes="<<mixes.size()<<" mix_title="<<mixCatalog.title<<"\n";
 return q.size()==88 && sup.size()==13 && x.guideId=="level-30-por-ela" && mixes.size()==127 && mixCatalog.totalCombinations==127 ? 0 : 8;
}
