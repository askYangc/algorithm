package interf

/* GetLen 获取当前结构体的大小 */
/* Pack 	入参 无，出参 结构体转换的二进制数据 */
/* Unpack 	入参 二进制数据，出参 偏移了二进制数据长度*/
type StructParser interface {
	GetLen()(uint32)
	Pack() ([]byte, error)
	Unpack([]byte) (uint32, error)
}