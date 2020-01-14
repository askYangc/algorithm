package util

import (
	"errors"
	"sync"
)

// Queue 是用于存放 int 的队列
type Queue struct {
	nums []interface{}
	sync.Mutex
	safe bool
}

// NewQueue 返回 *util.Queue
func NewQueue(safe bool) *Queue {
	return &Queue{nums: []interface{}{},safe:safe}
}

// Push 把 n 放入队列
func (q *Queue) Push(n interface{}) {
	if q.safe {
		q.Lock()
	}
	q.nums = append(q.nums, n)
	if q.safe {
		q.Unlock()
	}
}

// Pop 从 q 中取出最先进入队列的值
func (q *Queue) Pop() (interface{},error) {
	if q.safe {
		q.Lock()
	}
	if len(q.nums) == 0 {
		if q.safe {
			q.Unlock()
		}
		return nil, errors.New("empty")
	}
	res := q.nums[0]
	q.nums = q.nums[1:]
	if q.safe {
		q.Unlock()
	}
	return res, nil
}

// 只是取出来用，不删除
func (q *Queue) GetFront() (interface{},bool) {
	if q.safe {
		q.Lock()
	}
	if len(q.nums) == 0 {
		if q.safe {
			q.Unlock()
		}
		return nil, false
	}
	res := q.nums[0]
	if q.safe {
		q.Unlock()
	}
	return res, true
}


func (q *Queue) len() int {
	return len(q.nums)
}

// Len 返回 q 的长度
func (q *Queue) Len() int {
	if q.safe {
		q.Lock()
	}
	l := q.len()
	if q.safe {
		q.Unlock()
	}
	return l
}

// IsEmpty 反馈 q 是否为空
func (q *Queue) IsEmpty() bool {
	if q.safe {
		q.Lock()
	}
	res := true
	if q.len() == 0 {
		res = true
	}else {
		res = false
	}
	if q.safe {
		q.Unlock()
	}
	return res
}

