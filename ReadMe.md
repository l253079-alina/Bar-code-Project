# BarcodeSix

This is our group project. The app looks at a barcode picture, figures out the number hiding inside it, turns that into a 6-digit number, and then draws a fresh barcode for that number.

We wrote all the barcode reading and drawing ourselves, so there's no ready-made barcode library in here. Qt is only used for the window and drawing the picture. We built it with Qt 6 and C++.

## Who's working on what

Please only touch your own files so we don't end up with messy merge problems.

| Member | Files |
|---|---|
| 1 | `code128patterns.h`, `code128encoder.h` |
| 2 | `code128decoder.h` |
| 3 | `barcodeUtils.h`, `barcodeUtils.cpp` |
| 4 | `mainwindow.h`, `mainwindow.cpp` |
| 5 | `main.cpp`, `CMakeLists.txt`, `README.md`, `.gitignore` |

## What each part does

We agreed on these so all our pieces fit together. If you want to change any of them, please tell the team first.

- **Member 1** turns a string of digits into the black and white bars: `encodeDigits`
- **Member 2** reads a picture and gives back the number inside it, or nothing if it can't read it: `decodeGray`
- **Member 3** has three helpers:
  - `decodeBarcode` reads a barcode from an image
  - `toSixDigits` turns whatever text we get into our 6-digit number
  - `makeBarcodeImage` draws the barcode picture for a 6-digit number

## How we use Git

1. Member 5 uploads the first version to `main`, then everyone else copies it with `git clone <repo-url>`.
2. Make your own branch: `git checkout -b feature/<your-name>`
3. Change only your own files, then save your work with `git add <your files>` and `git commit -m "message"`.
4. Upload your branch with `git push -u origin feature/<your-name>` and open a Pull Request on GitHub.
5. Member 5 merges everyone's Pull Requests. After that, everyone runs `git checkout main` and `git pull` to get the latest version.

## How to run it

The easiest way is to open `CMakeLists.txt` in Qt Creator (you need Qt 6.4 or newer with the Widgets part installed).

If you like the terminal instead, run:

`cmake -B build && cmake --build build`

## Quick check that it works

1. Type `123456`
2. Click **Generate barcode**
3. Click **Save barcode PNG**
4. Click **Open image file** and pick the PNG you just saved

If everything works, `123456` should show up again.
