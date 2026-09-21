#include <cassert>

#include "gameplay/quests/v2/QuestObjective.h"

int main()
{
    using namespace gameplay::quests;

    QuestObjectiveDefinition kill;
    kill.objectiveId = 10;
    kill.type = QuestObjectiveType::Kill;
    kill.targetKey = "Hopy";
    kill.targetMap = 2;
    kill.requiredCount = 3;
    kill.allowPartyCredit = false;

    QuestObjectiveState progress;
    QuestObjectiveSignal signal;
    signal.type = QuestObjectiveType::Kill;
    signal.targetKey = "Hopy";
    signal.map = 2;
    signal.amount = 1;

    assert(QuestObjectiveEvaluator::Apply(kill, signal, progress));
    assert(progress.currentCount == 1);
    assert(!progress.IsComplete(kill));

    signal.partyCredit = true;
    assert(!QuestObjectiveEvaluator::Apply(kill, signal, progress));
    assert(progress.currentCount == 1);

    signal.partyCredit = false;
    signal.targetKey = "Goblin";
    assert(!QuestObjectiveEvaluator::Apply(kill, signal, progress));

    signal.targetKey = "Hopy";
    signal.amount = 5;
    assert(QuestObjectiveEvaluator::Apply(kill, signal, progress));
    assert(progress.currentCount == 3);
    assert(progress.IsComplete(kill));
    assert(!QuestObjectiveEvaluator::Apply(kill, signal, progress));

    QuestObjectiveDefinition gather;
    gather.type = QuestObjectiveType::Gather;
    gather.requiredCount = 2;
    gather.targetMap = -1;
    gather.targetKey.clear();

    QuestObjectiveSignal gathered;
    gathered.type = QuestObjectiveType::Gather;
    gathered.targetKey = "CopperOre";
    gathered.map = 7;
    gathered.amount = 2;

    QuestObjectiveState gatherProgress;
    assert(QuestObjectiveEvaluator::Apply(gather, gathered, gatherProgress));
    assert(gatherProgress.IsComplete(gather));
    return 0;
}
