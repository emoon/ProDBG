_PHONY: format

EXAMPLES = $(shell find examples -type f -name '*.rs')
SRCS = $(shell find src -type f -name '*.rs')

format: $(EXAMPLES) $(SRCS)
	@rustfmt --write-mode=overwrite $(EXAMPLES) $(SRCS)
