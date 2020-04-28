# ![Crow Translate logo](./dist/unix/generic/hicolor/72x72/apps/crow-translate.png) Crow Translate

[![GitHub (pre-)release](https://img.shields.io/github/release/crow-translate/crow-translate/all.svg)](https://github.com/crow-translate/crow-translate/releases)
[![Codacy grade](https://img.shields.io/codacy/grade/b28c6646bb324ffb98092f63a9b5896e.svg)](https://app.codacy.com/project/crow-translate/crow-translate/dashboard)

**Crow Translate** es un traductor simple y ligero programado en **C++ / Qt** que permite traducir y reproducir de manera hablada el texto utilizando las API de traducción de servicios como Google, Yandex o Bing.
También puedes estar interesado en mi biblioteca [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator "Una biblioteca para Qt5 que ofrece un libre uso de las API de traducción de Google, Yandex and Bing translate API.") Utilizada en este proyecto. 

## Índice

-   [Capturas de pantalla](#capturas-de-pantalla)
-   [Funcionalidades](#funcionalidades)
-   [Atajos de teclado predeterminados](#atajos-de-teclado-predeterminados)
-   [Comandos para la línea de comandos](#comandos-para-la-línea-de-comandos)
-   [API para D-Bus](#api-para-d-bus)
-   [Dependencias](#dependencias)
-   [Terceros](#terceros)
-   [Instalación](#instalación)
-   [Compilación](#compilación)
-   [Traducción](#traducción)

## Capturas de pantalla

### Linux KDE

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/main.png" alt="Captura de pantalla principal"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/popup.png" width="250px" height="140px" alt="Captura de pantalla de ventana emergente"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/sentense.png" width="250px" height="140px" alt="Captura de pantalla de una frase"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/settings.png" width="250px" height="140px" alt="Captura de pantalla de ajustes"/>
</p>
<p align="center">
  <sub><b>Capturas de pantalla</b>: SO: <a href="https://www.archlinux.org">Arch Linux</a> | Entorno de escritorio: <a href="https://www.kde.org/plasma-desktop">Plasma</a> | Tema: Breeze | Iconos: <a href="https://github.com/PapirusDevelopmentTeam/papirus-icon-theme">Papirus</a> | Fondo de escritorio: <a href="https://dynamicwallpaper.club/wallpaper/nrv0me8vd1">Catalina</a></sub>
</p>

### Windows 10

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/main.png" alt="Captura de pantalla principal"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/popup.png" width="250px" height="140px" alt="Captura de pantalla de ventana emergente"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/sentense.png" width="250px" height="140px" alt="Captura de pantalla de una frase"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/settings.png" width="250px" height="140px" alt="Captura de pantalla de ajustes"/>
</p>
<p align="center">
  <sub><b>Capturas de pantalla</b>: SO: <a href="https://www.microsoft.com/en-us/software-download/windows10">Windows 10</a> | Tema: predeterminado | Fondo de escritorio: <a href="https://wallpaperscraft.com/wallpaper/needles_spruce_branch_blur_114943">WallpapersCraft</a></sub>
</p>

## Funcionalidades

-   Traduce y reproduce de manera hablada el texto de cualquier aplicación que permita la selección de texto
-   Admite 117 idiomas diferentes
-   Bajo consumo de memoria (~20MB)
-   Atajos de teclado altamente personalizables
-   Interfaz para la línea de comandos con un buen número de opciones
-   API para D-Bus
-   Disponible para Linux y Windows

## Atajos de teclado predeterminados

Puedes cambiarlos en el apartado de ajustes. Algunas secuencias de teclas pueden no estar disponibles debido a limitaciones con el sistema operativo.

Wayland no permite el registro de atajos de teclado globales, pero puedes utilizar [D-Bus](#api-para-d-bus) para vincular acciones en los ajustes de sistema. Para entornos de escritorio que admitan [acciones de aplicaciones adicionales](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#extra-actions) (KDE, por ejemplo) los verás predefinidos en los ajustes de atajos de teclado del sistema. También puedes utilizarlos para sesiones X11, pero necesitas inhibir los registros de atajos globales en los ajustes de la aplicación para evitar conflictos.

### Global

| Combinación de teclas                           | Descripción                                              |
| ----------------------------------------------- | -------------------------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd> | Traducir el texto seleccionado                           |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>S</kbd> | Reproducir hablando el texto seleccionado                |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>F</kbd> | Reproducir hablando la traducción del texto seleccionado |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>G</kbd> | Parar la reproducción                                    |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>C</kbd> | Mostrar la pantalla principal                            |

### En la pantalla principal

| Combinación de teclas                             | Descripción                                                   |
| ------------------------------------------------- | ------------------------------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Enter</kbd>                | Traducir                                                      |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd>                    | Cerrar ventana                                                |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                    | Reproducir / pausar la reproducción hablando del texto origen |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>S</kbd> | Reproducir / pausar la reproducción hablando de la traducción |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> | Copiar la traducción al portapapeles                          |

## Comandos para la línea de comandos

El programa también tiene una interfaz para la consola.

**Uso:** `crow [opciones] texto`

| Opciones                   | Descripción                                                                                   |
| -------------------------- | ---------------------------------------------------------------------------------------------- |
| `-h, --help`               | Muestra la ayuda                                                                              |
| `-v, --version`            | Muestra la información de la versión                                                          |
| `-c, --codes`              | Muestra los códigos de los idiomas                                                                                          |
| `-a, --audio-only`         | Muestra el texto solo para reproducir hablando cuando se utiliza `--speak-translation` o `--speak-source`                       |
| `-s, --source <code>`      | Especifica el idioma original (de manera predeterminada, el motor de traducción intentará determinar el idioma por su cuenta) |
| `-t, --translation <code>` | Especifica el(los) idioma(s) de traducción separados por '+' (de manera predeterminada, es utilizado el idioma del sistema)    |
| `-l, --locale <code>`      | Especifica el idioma del traductor (de manera predeterminada, es utilizado el idioma del sistema                      |
| `-e, --engine <engine>`    | Especifica el motor de traducción del traductor ('google', 'yandex' o 'bing'), Google es utilizado de manera predeterminada  |
| `-p, --speak-translation`  | Reproduce hablando la traducción                                                                          |
| `-u, --speak-source`       | Reproduce hablando el texto origen                                                                               |
| `-f, --file`               | Lee el texto origen desde un archivo. Los argumentos serán interpretados como las rutas del archivo                             |
| `-i, --stdin`              | Añade los datos de stdin al texto origen                                                                  |

**Nota:** Si no le pasas argumentos al programa, se lanza la interfaz gráfica.

## API para D-Bus

Disponible actualmente solo para [atajos de teclado globales](#global).

    io.crow_translate.CrowTranslate
    └── /io/crow_translate/CrowTranslate/MainWindow
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copyTranslatedSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.open()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakTranslatedSelection()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.stopSpeaking()
        ├── method void io.crow_translate.CrowTranslate.MainWindow.translateSelection()
        └── method void io.crow_translate.CrowTranslate.MainWindow.quit()

Por ejemplo, puedes mostrar la ventana principal utilizando `dbus-send`:

```bash
dbus-send --type=method_call --dest=io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
```

O mediante `qdbus`:

```bash
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
# or shorter
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow open
```

## Dependencias

### Paquetes para Arch Linux

**Compilar:** `qt5-base qt5-multimedia qt5-x11extras qt5-tools`

**Ejecutar:** `qt5-base qt5-multimedia qt5-x11extras gst-plugins-good openssl`

### Paquetes para Debian

**Compilar:** `qt5-default qt5-qmake libqt5x11extras5-dev qttools5-dev-tools qtmultimedia5-dev qtbase5-dev qtbase5-dev-tools`

**Ejecutar**: `gstreamer1.0-fluendo-mp3 qtgstreamer-plugins-qt5 gstreamer1.0-plugins-good gstreamer1.0-alsa gstreamer1.0-pulseaudio libqt5multimedia5-plugins`

### Paquetes para openSUSE

**Compilar:**  `gcc-c++ libqt5-linguist-devel libqt5-qtbase-devel libqt5-qtmultimedia-devel libqt5-qtx11extras-devel`

### Paquetes para Solus 

**compilar:**  `qt5-tools-devel qt5-multimedia-devel qt5-svg-devel qt5-x11extras-devel`

**Ejecutar**: `qt5-base qt5-x11extras qt5-multimedia libstdc++ libgcc glibc libx11`

## Terceros

### Bibliotecas

Este proyecto utiliza las siguientes bibliotecas de terceros:

-   [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator) - ofrece el uso libre de las API de traducción de Google, Yandex y Bing.
-   [QGitTag](https://github.com/crow-translate/QGitTag) - utiliza la API de GitHub para ofrecer información sobre las nuevas versiones de la aplicación.
-   [QHotkey](https://github.com/Skycoder42/QHotkey) - ofrece atajos de teclado globales para las plataformas de escritorio.
-   [QTaskbarControl](https://github.com/Skycoder42/QTaskbarControl) - permite crear una barra de tareas/lanzador paratodas las plataformas de escritorio.
-   [SingleApplication](https://github.com/itay-grudev/SingleApplication) - previene la ejecución de múltiples instancias de la aplicación.

Además, si quieres clonar este proyecto, necesitarás utilizar la opción `--recursive`:

```bash
git clone --recursive git@github.com:crow-translate/crow-translate.git
```

o puedes inicializar posteriormente estos módulos:

```bash
git clone git@github.com:crow-translate/crow-translate.git
git submodule init
git submodule update
```

### Iconos

Solo en Linux se admite la modificación del icono. Windows utiliza los iconos [Papirus](https://github.com/PapirusDevelopmentTeam/papirus-icon-theme "Free and open source SVG icon theme").

Los iconos [FlagKit](https://github.com/madebybowtie/FlagKit "Beautiful flag icons for usage in apps and on the web") son utilizados para las banderas.

## Instalación

### Windows

:package: [Página de lanzamientos](https://github.com/crow-translate/crow-translate/releases/latest)

:package: [Paquetes Scoop](https://github.com/lukesampson/scoop-extras/blob/master/bucket/crow-translate.json)

```bash
sudo scoop install crow-translate -g
```

### Linux

#### Arch Linux, Manjaro, Chakra, etc

:package: [Versión estable en AUR](https://aur.archlinux.org/packages/crow-translate)

```bash
git clone https://aur.archlinux.org/crow-translate.git
cd crow-translate
makepkg -si
```

:package: [Versión de Git en AUR](https://aur.archlinux.org/packages/crow-translate-git)

```bash
git clone https://aur.archlinux.org/crow-translate-git.git
cd crow-translate-git
makepkg -si
```

#### Debian, Ubuntu, Mint, etc

:package: [Página de lanzamientos](https://github.com/crow-translate/crow-translate/releases/latest)

#### Fedora

:package: [Fedora Copr](https://copr.fedorainfracloud.org/coprs/faezebax/crow-translate)

```bash
sudo dnf copr enable faezebax/crow-translate
sudo dnf install crow-translate
```

#### CentOS, RHEL

:package: [Fedora Copr](https://copr.fedorainfracloud.org/coprs/faezebax/crow-translate)

```bash
sudo yum copr enable faezebax/crow-translate
sudo yum install crow-translate
```

#### openSUSE Tumbleweed

:package: [Repositorio de Tumbleweed](https://software.opensuse.org/package/crow-translate)

```bash
sudo zypper install crow-translate
```

#### openSUSE Leap

:package: [Open Build Service](https://software.opensuse.org/package/crow-translate)

#### Solus

:package: [Repositorio de Solus](https://dev.getsol.us/source/crow-translate)

```bash
sudo eopkg it crow-translate
```

## Compilación

### Script automático

Puedes utilizar el script automático que compila **Crow Translate** y crea un paquete para tu distribución:

```bash
cd dist/unix
./create-package.sh
```

Después ya puedes instalarlo de manera habitual en tu sistema. El script indicará donde estará el paquete despueś de compilarlo. Actualmente solo están admitidas las distribuciones **Arch Linux**, **Debian** y sus derivadas.

### Compilado manual

Puedes compilar **Crow Translate** utilizando los siguientes comandos:

```bash
qmake # O qmake-qt5 en algunas distribuciones
make
make clean
```

Obtendrás un binario llamado `crow`.

### Parámetros de compilado

-   `PORTABLE_MODE` - Habilita la funcionalidad de hacer el paquete portable. Si creas un archivo llamado `settings.ini` en la carpeta de la aplicación **Crow** guardará la configuración en él. También añade la opción de "Modo Portable" a los ajustes de la aplicación, que hace lo mismo.

Los parámetros de compilado se pasan a la hora de realizar qmake: `qmake "DEFINES += PORTABLE_MODE"`.

## Traducción

Para ayudar con la traducción puedes utilizar [Transifex](https://www.transifex.com/crow-translate/crow-translate) o traducir los archivos en `data/translations` con [Qt Linguist](https://doc.qt.io/Qt-5/linguist-translators.html) directamente. Para añadir un nuevo idioma, haz una petición en la página de Transifex o copia `data/translations/crow.ts` a `data/translations/crow_<ISO 639-1 código de idioma>.ts`, tradúcelo y haz un pull request en el repositorio de GitHub.
