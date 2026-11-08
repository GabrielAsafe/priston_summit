#pragma once
// SERVER-only. Monster spawn registration and one-time SQLite material award.
// Does not modify or mint any physical DAT items.
struct rsPLAYINFO;
namespace summit_gm396 {
void RegisterSpawn(unsigned int serial, const char* model, const char* display);
bool ProcessDeath(unsigned int serial, rsPLAYINFO* killer);
}
