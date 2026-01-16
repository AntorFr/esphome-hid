# ESPHome HID Mouse Component

Composant externe ESPHome permettant de simuler une souris USB HID sur ESP32-S3.

## Fonctionnalités

- Simulation complète d'une souris USB HID
- Mouvements relatifs (X, Y)
- Boutons gauche, droit et milieu
- Molette de défilement
- Compatible ESP32-S3 avec USB OTG natif

## Installation

Ajoutez le composant externe dans votre configuration ESPHome :

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/votre-username/esphome-hid
      ref: main
    components: [hid_mouse]
```

## Configuration

```yaml
esphome:
  name: esp32-hid-mouse
  platformio_options:
    board_build.flash_mode: dio

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf
    version: recommended

hid_mouse:
  id: my_mouse

# Exemple d'utilisation avec un bouton
binary_sensor:
  - platform: gpio
    pin: GPIO0
    name: "Click Button"
    on_press:
      - hid_mouse.click:
          id: my_mouse
          button: LEFT
```

## Actions disponibles

### `hid_mouse.move`
Déplace le curseur de manière relative.

```yaml
- hid_mouse.move:
    id: my_mouse
    x: 10
    y: -5
```

### `hid_mouse.click`
Effectue un clic avec le bouton spécifié.

```yaml
- hid_mouse.click:
    id: my_mouse
    button: LEFT  # LEFT, RIGHT, MIDDLE
```

### `hid_mouse.press`
Appuie sur un bouton (sans relâcher).

```yaml
- hid_mouse.press:
    id: my_mouse
    button: LEFT
```

### `hid_mouse.release`
Relâche un bouton.

```yaml
- hid_mouse.release:
    id: my_mouse
    button: LEFT
```

### `hid_mouse.scroll`
Fait défiler la molette.

```yaml
- hid_mouse.scroll:
    id: my_mouse
    amount: 1  # positif = haut, négatif = bas
```

## Prérequis

- ESP32-S3 avec support USB OTG natif
- ESPHome 2023.12.0 ou plus récent
- Framework ESP-IDF

## Licence

MIT License - voir [LICENSE](LICENSE)
