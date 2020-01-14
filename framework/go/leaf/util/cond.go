package util

type SingleCond chan struct{}

func MakeSingleCond() SingleCond {
	return make(SingleCond, 1)
}

//阻塞线程，直到获取一个许可证
func (s SingleCond) Notify() {
	select {
		case s <- struct{}{}:
		default:
	}
}

//增加许可证，会释放一个阻塞的acquire方法
func (s SingleCond) Wait() {
	<-s
}

func (s SingleCond) GetSignelCond() (<-chan struct{}) {
	return s
}

