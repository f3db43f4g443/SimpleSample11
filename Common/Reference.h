#pragma once

#define SAFE_RELEASE(x) {if(x) {(x)->Release(); (x) = NULL;}}

class CReferenceObject {
public:
	CReferenceObject() { m_refCount = 0; }
	virtual ~CReferenceObject() {}
	void AddRef() { m_refCount++; }
	void Release() { m_refCount--; if( !m_refCount ) Dispose(); }
protected:
	virtual void Dispose() { delete this; }
private:
	int m_refCount;
};

template<class T> class CReference {
public:
	CReference() { m_ptr = NULL; }
	CReference( const struct SClassCreateContext& context ) { if( m_ptr ) m_ptr->AddRef(); }
	CReference( const CReference<T>& r ) { m_ptr = r.m_ptr; if( m_ptr ) m_ptr->AddRef(); }
	CReference( T* p ) { m_ptr = p; if( m_ptr ) m_ptr->AddRef(); }
	~CReference() { SAFE_RELEASE( m_ptr ); }

	CReference& operator = ( T* p ) {
		T* orig = m_ptr;
		m_ptr = p;
		if( m_ptr ) m_ptr->AddRef();
		if( orig ) orig->Release();
		return *this;
	}
	CReference& operator = ( const CReference& r ) {
		T* orig = m_ptr;
		m_ptr = r.m_ptr;
		if( m_ptr ) m_ptr->AddRef();
		if( orig ) orig->Release();
		return *this;
	}
	T& operator * () const {
		return *m_ptr;
	}
	T* operator -> () const {
		return m_ptr;
	}
	T** operator & () {
		return &m_ptr;
	}
	operator T* () const {
		return m_ptr;
	}
	operator bool() const {
		return m_ptr != NULL;
	}
	/*bool operator == (const CReference& r) const {
		return m_ptr == r.m_ptr;
		}*/
	bool operator < ( const CReference& r ) const {
		return m_ptr < r.m_ptr;
	}
	bool operator >( const CReference& r ) const {
		return m_ptr > r.m_ptr;
	}
	bool operator <= ( const CReference& r ) const {
		return m_ptr <= r.m_ptr;
	}
	bool operator >= ( const CReference& r ) const {
		return m_ptr >= r.m_ptr;
	}
	bool operator != ( const CReference& r ) const {
		return m_ptr != r.m_ptr;
	}

	T* GetPtr() { return m_ptr; }
	T** AssignPtr() { SAFE_RELEASE( m_ptr ); return &m_ptr; }
private:
	T* m_ptr;
};