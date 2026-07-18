#ifndef KEK_EXAMPLES_GAME_HOOK_SUPPORT_H
#define KEK_EXAMPLES_GAME_HOOK_SUPPORT_H

#include "examples/game/game_app.h"
#include "examples/game/hooks/game_state_hooks.h"

typedef const Enemy* (*DeclaredEnemyReadFn)(const Game_stateRuntime* runtime);
typedef int (*DeclaredEnemyUpdateFn)(Game_stateRuntime* runtime,
                                     KekStateStorageUpdateFn update,
                                     void* context,
                                     uint64_t changed_fields);

int move_declared_enemy(KekHookContext* context, DeclaredEnemyReadFn read_enemy,
                        DeclaredEnemyUpdateFn update_enemy, float speed, float strafe);

#endif
