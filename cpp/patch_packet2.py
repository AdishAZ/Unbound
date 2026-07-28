with open('d:/Unbound/pokemon/cpp/src/network/packet.cpp', 'r') as f:
    content = f.read()

content = content.replace('s.GetData()', 's.GetBuffer()')
content = content.replace('s.WriteUint64', 's.WriteU64')
content = content.replace('s.ReadUint64', 's.ReadU64')
content = content.replace('s.WriteUint32', 's.WriteU32')
content = content.replace('s.ReadUint32', 's.ReadU32')

content = content.replace('s.WriteInt64(c.play_time_seconds);', 's.WriteU64(static_cast<uint64_t>(c.play_time_seconds));')
content = content.replace('c.play_time_seconds = s.ReadInt64();', 'c.play_time_seconds = static_cast<int64_t>(s.ReadU64());')

content = content.replace('s.WriteInt64(c.last_login);', 's.WriteU64(static_cast<uint64_t>(c.last_login));')
content = content.replace('c.last_login = s.ReadInt64();', 'c.last_login = static_cast<int64_t>(s.ReadU64());')

with open('d:/Unbound/pokemon/cpp/src/network/packet.cpp', 'w') as f:
    f.write(content)
