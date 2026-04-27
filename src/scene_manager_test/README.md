# Scene Manager Integration Test

Dieses Projekt testet den `core::SceneManager` aus dem Engine-Core gemäß [Issue #4](https://github.com/palle34/gba-engine/issues/4).

## Ziel

Verifizierung, dass das **RAII-Muster** korrekt funktioniert:  
Beim Wechsel einer Szene müssen alle zugehörigen Objekte (Sprites, eingebettete Spielobjekte wie `Player`) physisch aus dem Speicher entfernt werden, **bevor** die neue Szene initialisiert wird.

---

## Szenen

| Szene | Beschreibung |
|---|---|
| `TitleScene` | Startszene. Zeigt einen Titelbildschirm. START wechselt zur `LevelScene`. |
| `LevelScene` | Spielszene. Besitzt ein `Player`-Objekt (RAII-Nachweis). START wechselt zurück zur `TitleScene`. |

`Player` ist ein reines Testobjekt innerhalb der `LevelScene`, das nur `BN_LOG`-Ausgaben in seinem Konstruktor und Destruktor produziert.

---

## Aufbau von `main.cpp`

`main()` ist bewusst minimal gehalten – nur der SceneManager-Lifecycle läuft hier:

```cpp
int main()
{
    bn::core::init();
    // ...

    core::SceneManager::instance().set_next_scene(bn::make_unique<TitleScene>(gen));

    while(true)
    {
        core::SceneManager::instance().update();
        bn::core::update();
    }
}
```

---

## Verifikation in mGBA

1. ROM bauen (siehe unten) und in **mGBA** laden.
2. **Tools → Message Log** öffnen.
3. **START** drücken, um zwischen den Szenen zu wechseln.

### Erwartete Log-Ausgabe (TitleScene → LevelScene)

```
CONSTRUCTOR: TitleScene initialized.
--- START gedrückt ---
CONSTRUCTOR: Player created.
CONSTRUCTOR: LevelScene initialized.
DESTRUCTOR:  TitleScene deleted.
```

### Erwartete Log-Ausgabe (LevelScene → TitleScene)

```
CONSTRUCTOR: TitleScene initialized.
DESTRUCTOR:  Player removed from memory.
DESTRUCTOR:  LevelScene deleted.
```

> **Wichtig:** Die `DESTRUCTOR`-Logs der alten Szene müssen **nach** dem `CONSTRUCTOR`-Log der neuen Szene erscheinen.  
> Das ist das erwartete Verhalten des `SceneManager` (`set_next_scene` konstruiert zuerst, `update()` löscht dann die alte Szene via RAII).

---

## Akzeptanzkriterien (aus Issue #4)

- [x] `main.cpp` beschränkt sich auf die Initialisierung des SceneManagers
- [x] Wechsel von `TitleScene` → `LevelScene` und zurück funktioniert ohne Absturz
- [x] Das mGBA-Log bestätigt die Zerstörung der alten Szene und ihres `Player`-Objekts
- [ ] VRAM-Viewer in mGBA zeigt, dass alte Tiles verschwinden und durch neue ersetzt werden (kein „Tile-Stacking")

---

## Build

```bash
cd src/scene_manager_test
make
```

> Voraussetzung: Butano-Devcontainer mit `DEVKITARM` und `WONDERFUL_TOOLCHAIN`.  
> Siehe das [Haupt-README](../../README.md) für Setup-Anweisungen.
