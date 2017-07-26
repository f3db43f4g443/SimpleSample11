#pragma once

#define SAFE_RELEASE(x) {if(x) {(x)->Release(); (x) = NULL;}}

class CReferenceObject {
public:
	FORCE_INLINE CReferenceObject() { m_refCount = 0; }
	virtual ~CReferenceObject() {}
	FORCE_INLINE void AddRef() { m_refCount++; }
	FORCE_INLINE void Release() { m_refCount--; if( !m_refCount ) Dispose(); }
protected:
	virtual void Dispose() { delete this; }
private:
	int m_refCount;
};

template<class T> class CReference {
public:
	FORCE_INLINE CReference() { m_ptr = NULL; }
	FORCE_INLINE CReference( const struct SClassCreateContext& context ) { if( m_ptr ) m_ptr->AddRef(); }
	FORCE_INLINE CReference( const CReference<T>& r ) { m_ptr = r.m_ptr; if( m_ptr ) m_ptr->AddRef(); }
	FORCE_INLINE CReference( T* p ) { m_ptr = p; if( m_ptr ) m_ptr->AddRef(); }
	FORCE_INLINE ~CReference() { SAFE_RELEASE( m_ptr ); }

	FORCE_INLINE CReference& operator = ( T* p ) {
		T* orig = m_ptr;
		m_ptr = p;
		if( m_ptr ) m_ptr->AddRef();
		if( orig ) orig->Release();
		return *this;
	}
	FORCE_INLINE CReference& operator = ( const CReference& r ) {
		T* orig = m_ptr;
		m_ptr = r.m_ptr;
		if( m_ptr ) m_ptr->AddRef();
		if( orig ) orig->Release();
		return *this;
	}
	FORCE_INLINE T& operator * () const {
		return *m_ptr;
	}
	FORCE_INLINE T* operator -> () const {
		return m_ptr;
	}
	FORCE_INLINE T** operator & () {
		return &m_ptr;
	}
	FORCE_INLINE operator T* () const {
		return m_ptr;
	}
	FORCE_INLINE operator bool() const {
		return m_ptr != NULL;
	}
	/*bool operator == (const CReference& r) const {
		return m_ptr == r.m_ptr;
		}*/
	FORCE_INLINE bool operator < ( const CReference& r ) const {
		return m_ptr < r.m_ptr;
	}
	FORCE_INLINE bool operator >( const CReference& r ) const {
		return m_ptr > r.m_ptr;
	}
	FORCE_INLINE bool operator <= ( const CReference& r ) const {
		return m_ptr <= r.m_ptr;
	}
	FORCE_INLINE bool operator >= ( const CReference& r ) const {
		return m_ptr >= r.m_ptr;
	}
	/*bool operator != ( const CReference& r ) const {
		return m_ptr != r.m_ptr;
	}*/

	FORCE_INLINE T* GetPtr() const { return m_ptr; }
	FORCE_INLINE T** AssignPtr() { SAFE_RELEASE( m_ptr ); return &m_ptr; }
private:
	T* m_ptr;
};