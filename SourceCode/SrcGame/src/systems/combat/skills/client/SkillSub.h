struct sSKILL;
class smCHAR;



int OpenPlaySkill(sSKILL *lpSkill);

int PlaySkillAttack(sSKILL *lpSkill, smCHAR *lpChar);

int PlaySkillSubLife(int Life);

int GetSkillDistRange(sSKILL *lpSkill, int AttackRange, int AttackMode);

int PlaySkillLoop(int SkillCode, smCHAR *lpChar);


int SetSkillMotionBlurColor(int dwSkillCode);

int GetSkillPower(smCHAR *lpChar, smCHAR *lpChar2, int dx, int dy, int dz);

int	LearnSkillEffect(smCHAR *lpChar = 0);

int CheckPlaySkill();
void SummitWhirlwindTick();
void SummitArcherBombardmentTick();
void SummitMagicianCataclysmTick();
void SummitT5PersistentParticleTick();

// Tier 5 MotionEvent bridge: animation/event identity is separate from AttackSkil/network routing.
void SummitT5MotionEventArm(smCHAR *actor, int logicalSkillCode, int networkSkillCode, int level, smCHAR *target, int finalEvent);
void SummitT5MotionEventClear(smCHAR *actor);
void SummitT5MotionEventEnd(smCHAR *actor);
int SummitT5MotionEventDispatch(smCHAR *actor);




