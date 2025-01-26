#!/bin/bash
#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#

POTFILE=di.pot

xgettext -s -d bdj4 \
  --from-code=UTF-8 \
  --language=C \
  --add-comments=CONTEXT: \
  --no-location \
  --keyword=DI_GT \
  --flag=_:1:pass-c-format \
  di.c \
  -p po -o ${POTFILE}
