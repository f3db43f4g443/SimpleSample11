#ifndef _LINKLIST_H_
#define _LINKLIST_H_


#define LINK_LIST( type, name ) \
	protected: \
		type** __pPrev##name; \
		type* __pNext##name; \
	public: \
		void InsertAfter_##name( type* pInserted ) \
		{ \
			pInserted->__pPrev##name = &__pNext##name; \
			pInserted->__pNext##name = __pNext##name; \
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = &pInserted->__pNext##name; \
			} \
			__pNext##name = pInserted; \
		} \
		void InsertBefore_##name( type* pInserted ) \
		{ \
			pInserted->InsertTo_##name( *__pPrev##name ); \
		} \
		void InsertTo_##name( type* &pHead ) \
		{ \
			if( pHead ) \
			{ \
				pHead->__pPrev##name = &__pNext##name; \
			} \
			__pNext##name = pHead; \
			__pPrev##name = &pHead; \
			pHead = this; \
		} \
		type** Transplant_##name( type* &pHead ) \
		{ \
			auto ppRet = __pPrev##name; \
			*__pPrev##name = pHead; \
			if( pHead ) \
				pHead->__pPrev##name = __pPrev##name; \
			__pPrev##name = &pHead; \
			pHead = this; \
			return ppRet; \
		} \
		static void Swap_##name( type* a, type* b, type* c ) \
		{ \
			type* pHead = NULL; \
			type** ppHead1 = a->Transplant_##name( pHead ); \
			ppHead1 = b->Transplant_##name( *ppHead1 ); \
			ppHead1 = c->Transplant_##name( *ppHead1 ); \
			a->Transplant_##name( *ppHead1 ); \
		} \
		void RemoveFrom_##name() \
		{ \
			*__pPrev##name = __pNext##name;\
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = __pPrev##name;\
			} \
		} \
		void RemoveFrom_##name##_itr() \
		{ \
			*__pPrev##name = __pNext##name;\
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = __pPrev##name;\
			} \
		} \
		type* Shift_##name() \
		{ \
			type* pNext = __pNext##name; \
			*__pPrev##name = pNext; \
			if( pNext ) \
			{ \
				pNext->__pPrev##name = __pPrev##name;\
				__pPrev##name = &pNext->__pNext##name; \
				__pNext##name = pNext->__pNext##name; \
				if( __pNext##name ) \
				{ \
					__pNext##name->__pPrev##name = &__pNext##name; \
				} \
				pNext->__pNext##name = this; \
				return pNext; \
			} \
			else \
			{ \
				return NULL; \
			} \
		} \
		type* &Next##name() \
		{ \
			return __pNext##name; \
		}

#define LINK_LIST_REF( type, name ) \
	protected: \
		type** __pPrev##name; \
		type* __pNext##name; \
	public: \
		void InsertAfter_##name( type* pInserted ) \
		{ \
			pInserted->AddRef(); \
			pInserted->__pPrev##name = &__pNext##name; \
			pInserted->__pNext##name = __pNext##name; \
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = &pInserted->__pNext##name; \
			} \
			__pNext##name = pInserted; \
		} \
		void InsertBefore_##name( type* pInserted ) \
		{ \
			pInserted->InsertTo_##name( *__pPrev##name ); \
		} \
		void InsertTo_##name( type* &pHead ) \
		{ \
			AddRef(); \
			if( pHead ) \
			{ \
				pHead->__pPrev##name = &__pNext##name; \
			} \
			__pNext##name = pHead; \
			__pPrev##name = &pHead; \
			pHead = this; \
		} \
		type** Transplant_##name( type* &pHead ) \
		{ \
			auto ppRet = __pPrev##name; \
			*__pPrev##name = pHead; \
			if( pHead ) \
				pHead->__pPrev##name = __pPrev##name; \
			__pPrev##name = &pHead; \
			pHead = this; \
			return ppRet; \
		} \
		static void Swap_##name( type* a, type* b, type* c ) \
		{ \
			type* pHead = NULL; \
			type** ppHead1 = a->Transplant_##name( pHead ); \
			ppHead1 = b->Transplant_##name( *ppHead1 ); \
			ppHead1 = c->Transplant_##name( *ppHead1 ); \
			a->Transplant_##name( *ppHead1 ); \
		} \
		void RemoveFrom_##name() \
		{ \
			*__pPrev##name = __pNext##name;\
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = __pPrev##name;\
			} \
			Release(); \
		} \
		void RemoveFrom_##name##_itr() \
		{ \
			*__pPrev##name = __pNext##name;\
			if( __pNext##name ) \
			{ \
				__pNext##name->__pPrev##name = __pPrev##name;\
			} \
		} \
		type* Shift_##name() \
		{ \
			type* pNext = __pNext##name; \
			*__pPrev##name = pNext; \
			if( pNext ) \
			{ \
				pNext->__pPrev##name = __pPrev##name;\
				__pPrev##name = &pNext->__pNext##name; \
				__pNext##name = pNext->__pNext##name; \
				if( __pNext##name ) \
				{ \
					__pNext##name->__pPrev##name = &__pNext##name; \
				} \
				pNext->__pNext##name = this; \
				return pNext; \
			} \
			else \
			{ \
				return NULL; \
			} \
		} \
		type* &Next##name() \
		{ \
			return __pNext##name; \
		}

#define LINK_LIST_HEAD( member, type, name ) \
	protected: \
		type* member; \
	public: \
		type*& Get_##name() \
		{ \
			return member; \
		} \
		void Insert_##name( type* pInserted ) \
		{ \
			pInserted->InsertTo_##name( member ); \
		} \
		void Remove_##name( type* pRemoved ) \
		{ \
			pRemoved->RemoveFrom_##name(); \
		}

#define LINK_LIST_REF_HEAD( member, type, name ) LINK_LIST_HEAD( member, type, name )

#define LINK_LIST_HEAD_ARR( member, size, type, name ) \
	protected: \
		type* member[size]; \
	public: \
		type*& Get_##name( int n ) \
		{ \
			return member[n]; \
		} \
		void Insert_##name( type* pInserted, int n ) \
		{ \
			pInserted->InsertTo_##name( member[n] ); \
		} \
		void Remove_##name( type* pRemoved, int n ) \
		{ \
			pRemoved->RemoveFrom_##name(); \
		}

#define LINK_LIST_REF_HEAD_ARR( member, size, type, name ) LINK_LIST_HEAD_ARR( member, size, type, name )

#define LINK_LIST_FOR_EACH_BEGIN( var, head, type, name ) \
	{ \
		if( head ) \
		{ \
			type* itr = (type*)alloca( sizeof( type ) ); \
			memset( itr, 0, sizeof( type ) ); \
			itr->InsertTo_##name( head ); \
			type* var; \
			for( var = itr->Shift_##name(); var; var = itr->Shift_##name() ) { \

#define LINK_LIST_FOR_EACH_END( var, head, type, name ) \
			} \
			if( var ) \
				itr->RemoveFrom_##name##_itr(); \
		} \
	}

#endif
