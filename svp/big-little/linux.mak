
# target source
OBJS := $(SRCS:%.c=%.o)

.PHONY : clean all

all: $(TARGET)

$(TARGET):$(COMM_OBJ) $(SAMPLE_CLIENT_OBJ) $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ $(MPI_LIBS) $(REL_LIB)/libsecurec.a

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJS)
	@rm -f $(SAMPLE_CLIENT_OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
