# Tiny Dungeon: States, Hooks, and Transitions

This diagram shows how runtime events trigger hooks, how hooks mutate state, and when state change notifications are published.

```mermaid
flowchart TD
  RT[KekRuntime event loop]
  E1[KEK_EVENT_STREAM_DATA]
  E2[KEK_EVENT_STREAM_ERROR]
  H1[stream_data_handler]
  H2[stream_error_handler]

  RT --> E1 --> H1
  RT --> E2 --> H2

  subgraph GS[GameState]
    SI[StandardInput]
    SO[StandardOutput]
    P[Player]
    D[Dungeon]
  end

  H1 --> I1[app_set_input_state]
  I1 --> SI
  I1 --> PUB1[publish state changed: StandardInput]
  H1 --> LOG[write keyboard_log]

  H1 --> DEC{Key decode}
  DEC -->|Arrow keys ESC sequence| AR[game_handle_arrow]
  DEC -->|W/A/S/D or R/Q| KH[game_handle_key]

  AR --> MV[game_move]
  KH --> MV
  KH -->|R| RST[game_reset]
  KH -->|Q| QUIT[request_quit]

  subgraph GT[Dungeon progression]
    RUN[Running]
    WIN[Game Over: Won]
    LOSE[Game Over: Lost]
  end

  MV --> RUN
  RUN -->|Move into wall| RUN
  RUN -->|Pick treasure| RUN
  RUN -->|Fight/adjacent goblin damage| RUN
  RUN -->|Reach exit with treasure| WIN
  RUN -->|Health <= 0| LOSE
  RST --> RUN

  MV --> PUB2[game_publish_state]
  RST --> PUB2
  KH -->|invalid key or Q message| PUB2
  PUB2 --> P
  PUB2 --> D
  PUB2 --> PUB3[publish state changed: Player and Dungeon]

  MV --> RENDER[game_render]
  RST --> RENDER
  KH -->|invalid key| RENDER
  RENDER --> OUT[app_write_raw]
  OUT --> O1[app_track_output_state]
  O1 --> SO
  O1 --> PUB4[publish state changed: StandardOutput]

  H2 --> EQ{source == stdin?}
  EQ -->|yes| QUIT
  EQ -->|no| NOOP[ignore]
```

## Notes

- `StandardInput` is updated for every incoming stream data event.
- `Player` and `Dungeon` are published together via `game_publish_state`.
- `StandardOutput` is updated as part of rendering/write calls.
- Restart (`R`) resets gameplay state while preserving stream-backed I/O state.
