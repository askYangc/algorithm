package util

type Semaphore chan struct{}

func MakeSemaphore(n int) Semaphore {
	return make(Semaphore, n)
}

//阻塞线程，直到获取一个许可证
func (s Semaphore) Acquire() {
	s <- struct{}{}
}

//增加许可证，会释放一个阻塞的acquire方法
func (s Semaphore) Release() {
	<-s
}

func (s Semaphore) GetSem() (<-chan struct{}) {
	return s
}

