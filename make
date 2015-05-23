set -e

clang -o rendertext rendertext.c `pkg-config --cflags --libs freetype2` -lX11 -lXrender

./rendertext
