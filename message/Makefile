-include def.mk

all:./ProtoDefine.h

./ProtoDefine.h:$(FILE_HEAD)
	echo "#ifndef _PROTODEFINE_H_" > ./ProtoDefine.h
	echo "#define _PROTODEFINE_H_" >> ./ProtoDefine.h
	find . -iname "*.h" | awk '{print "basename",$$0}' | sh | sort | awk '{if($$0!~/ProtoDefine/)printf("#include \"%s\"\n",$$0);}' >> ./ProtoDefine.h
	echo "#endif //_PROTODEFINE_H_" >> ./ProtoDefine.h

./%.pb.h:../message_new_proto/%.proto
	protoc --proto_path=../message_new_proto --cpp_out=./ $<

./chat/%.pb.h:../message_new_proto/chat/%.proto
	protoc --proto_path=../message_new_proto/chat --cpp_out=./chat $<

./logic/%.pb.h:../message_new_proto/logic/%.proto
	protoc --proto_path=../message_new_proto/logic --cpp_out=./logic $<

./map/%.pb.h:../message_new_proto/map/%.proto
	protoc --proto_path=../message_new_proto/map --cpp_out=./map $<

./log/%.pb.h:../message_new_proto/log/%.proto
	protoc --proto_path=../message_new_proto/log --cpp_out=./log $<

clean:
	$(RM) -r ProtoDefine.h def.mk chat/* logic/* map/* log/* *.pb.h *.pb.cc
