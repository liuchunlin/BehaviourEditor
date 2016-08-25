#pragma once
#include "Types.h"

namespace Vek
{
	// for dynamic cast
	class TreeNodeBase
	{
	public:
		virtual ~TreeNodeBase(){}
	};

	// TNode must be derived class of TreeNode
	template <class TNode>
	class TreeNode : public TreeNodeBase
	{
	public:
		TreeNode()
			:m_pParent(NULL)
			,m_pPrev(NULL)
			,m_pNext(NULL)
			,m_pFirstChild(NULL)
		{
		}
		virtual ~TreeNode()
		{
			if (m_pFirstChild)
			{
				TNode* pChild = m_pFirstChild;
				while (pChild)
				{
					TNode* pNext = pChild->m_pNext;
					delete pChild;
					pChild = pNext;
				}
				m_pFirstChild = NULL;
			}
		}

		TNode* Parent() const { return m_pParent; }
		TNode* Prev() const { return m_pPrev; }
		TNode* Next() const { return m_pNext; }
		TNode* FirstChild() const { return m_pFirstChild; }
		TNode* LastChild() const;
		void AddChild( TNode* pNode );
		bool InsertBefore( TNode* pNode, TNode* pNodeReferTo );
		bool InsertAfter( TNode* pNode, TNode* pNodeReferTo );
		bool InsertOrAppend( TNode* pNode, TNode* pNodeReferTo );
		void Unlink();
		void Remove( bool bRemoveNexts = false );
		void RemoveChildren();
		static bool Swap( TNode* pLeft, TNode *pRight );

		bool IsParent( TNode* pNode ) const;
		template <class Pr>
		TNode* FindFirstChild(const Pr& predicate, bool bRecursive = false);
		template <class Pr>
		TNode* FindLastChild(const Pr& predicate, bool bRecursive = false);
		template <class Pr>
		void FindChildren(const Pr& predicate, vector<TNode*>& outChildren, bool bRecursive = false);
		template <class Pr>
		TNode* FindParent(const Pr& predicate);

	private:
		TNode*		m_pParent;
		TNode*		m_pPrev;
		TNode*		m_pNext;
		TNode*		m_pFirstChild;
	};

	template <class TNode>
	TNode* TreeNode<TNode>::LastChild() const
	{
		if( m_pFirstChild == NULL )
			return NULL;
		TNode* pLast = m_pFirstChild;
		for( ; pLast->m_pNext; pLast = pLast->m_pNext );

		return pLast;
	}

	template <class TNode>
	void TreeNode<TNode>::AddChild( TNode* pNode )
	{
		if( pNode == NULL )
			return;
		pNode->m_pParent = (TNode*)this;
		if ( m_pFirstChild != NULL )
		{
			TNode* pPrev = LastChild();
			if( pPrev != pNode )
			{
				pNode->m_pPrev = pPrev;
				pPrev->m_pNext = pNode;
				pNode->m_pNext = NULL;
			}
		}
		else
		{
			m_pFirstChild = pNode;
			pNode->m_pPrev = NULL;
			pNode->m_pNext = NULL;
		}
	}

	template <class TNode>
	bool TreeNode<TNode>::InsertBefore( TNode* pNode, TNode* pNodeReferTo )
	{
		assert(pNode != pNodeReferTo);
		assert(pNodeReferTo == NULL || pNodeReferTo->m_pParent == this);

		if(pNode == NULL)
			return false;

		TNode* prevNode = pNodeReferTo ? pNodeReferTo->m_pPrev : LastChild();
		pNode->m_pParent = (TNode*)this;
		pNode->m_pPrev = prevNode;
		pNode->m_pNext = pNodeReferTo;
		if(prevNode != NULL)
		{
			prevNode->m_pNext = pNode;
		}
		if(pNodeReferTo != NULL)
		{
			pNodeReferTo->m_pPrev = pNode;
		}

		if(prevNode == NULL)
		{
			m_pFirstChild = pNode;
		}

		return true;
	}

	template <class TNode>
	bool TreeNode<TNode>::InsertAfter( TNode* pNode, TNode* pNodeReferTo )
	{
		assert(pNode != pNodeReferTo);
		assert(pNodeReferTo == NULL || pNodeReferTo->m_pParent == this);

		if( pNode == NULL )
			return false;

		TNode* nextNode = pNodeReferTo ? pNodeReferTo->m_pNext : m_pFirstChild;
		pNode->m_pParent = (TNode*)this;
		pNode->m_pPrev = pNodeReferTo;
		pNode->m_pNext = nextNode;
		if( nextNode != NULL )
		{
			nextNode->m_pPrev = pNode;
		}
		if( pNodeReferTo != NULL )
		{
			pNodeReferTo->m_pNext = pNode;
		}
		if( pNodeReferTo == NULL )
		{
			m_pFirstChild = pNode;
		}

		return true;
	}

	template <class TNode>
	bool TreeNode<TNode>::InsertOrAppend( TNode* pNode, TNode* pNodeReferTo )
	{
		return InsertAfter(pNode, pNodeReferTo ? pNodeReferTo : LastChild());
	}

	template <class TNode>
	void TreeNode<TNode>::Unlink()
	{
		if( m_pPrev != NULL )
		{
			m_pPrev->m_pNext = m_pNext;
		}
		else if( m_pParent != NULL )
		{
			m_pParent->m_pFirstChild = m_pNext;
		}
		if( m_pNext != NULL )
		{
			m_pNext->m_pPrev = m_pPrev;
		}

		m_pParent = NULL;
		m_pPrev = NULL;
		m_pNext = NULL;
	}

	template <class TNode>
	void TreeNode<TNode>::Remove( bool bRemoveNexts )
	{
		if( m_pPrev != NULL )
		{
			m_pPrev->m_pNext = m_pNext;
		}
		if( m_pNext != NULL )
		{
			m_pNext->m_pPrev = m_pPrev;
		}
		if( m_pParent != NULL && m_pParent->m_pFirstChild == this )
		{
			m_pParent->m_pFirstChild = m_pNext;
		}
		TNode* pNext = m_pNext;
		m_pParent = NULL;
		m_pPrev = NULL;
		m_pNext = NULL;
		delete this;

		if( bRemoveNexts && pNext )
		{
			pNext->Remove( true );
		}
	}

	template <class TNode>
	void TreeNode<TNode>::RemoveChildren()
	{
		if (m_pFirstChild)
		{
			m_pFirstChild->Remove(true);
		}
	}
	
	template <class TNode>
	bool TreeNode<TNode>::Swap( TNode* pLeft, TNode *pRight )
	{
		assert( pLeft != NULL && pRight != NULL );
		if( pLeft == NULL || pRight == NULL )
			return false;

		TNode* pLeftParent = pLeft->m_pParent;
		TNode* pRightParent = pRight->m_pParent;
		TNode* pLeftPrev = pLeft->m_pPrev;
		TNode* pRightPrev = pRight->m_pPrev;

		pLeft->Unlink();
		pRight->Unlink();

		if (pLeftParent)
		{
			pLeftParent->InsertAfter(pRight, pLeftPrev);
		}
		if (pRightParent)
		{
			pRightParent->InsertAfter(pLeft, pRightPrev);
		}

		return true;
	}

	template <class TNode>
	bool TreeNode<TNode>::IsParent( TNode* pNode ) const
	{
		if( pNode == NULL )
			return false;

		TNode* pParent = Parent();
		while ( pParent != NULL  )
		{
			if( pParent == pNode )
				return true;
			pParent = pParent->Parent();
		}
		return false;
	}

	template <class TNode>
	template<class Pr>
	TNode* TreeNode<TNode>::FindFirstChild(const Pr& predicate, bool bRecursive)
	{
		for (TNode* pChild = m_pFirstChild; pChild != NULL; pChild = pChild->m_pNext)
		{
			if( predicate( pChild ) )
			{
				return pChild;
			}
			if (bRecursive)
			{
				if (TNode* pResult = pChild->FindFirstChild(predicate, bRecursive))
				{
					return pResult;
				}
			}
		}
		return NULL;
	}

	template <class TNode>
	template<class Pr>
	TNode* TreeNode<TNode>::FindLastChild(const Pr& predicate, bool bRecursive)
	{
		for (TNode* pChild = LastChild(); pChild != NULL; pChild = pChild->m_pPrev)
		{
			if (bRecursive)
			{
				if (TNode* pResult = pChild->FindLastChild(predicate, bRecursive))
				{
					return pResult;
				}
			}
			if (predicate(pChild))
			{
				return pChild;
			}
		}
		return NULL;
	}

	template <class TNode>
	template<class Pr>
	void TreeNode<TNode>::FindChildren(const Pr& predicate, vector<TNode*>& outChildren, bool bRecursive)
	{
		for (TNode* pChild = m_pFirstChild; pChild != NULL; pChild = pChild->m_pNext)
		{
			if( predicate( pChild ) )
			{
				outChildren.push_back(pChild);
			}
			if (bRecursive)
			{
				pChild->FindChildren(predicate, outChildren, bRecursive);
			}
		}
	}

	template <class TNode>
	template<class Pr>
	TNode* TreeNode<TNode>::FindParent(const Pr& predicate)
	{
		for (TNode* pParent = Parent(); pParent != NULL; pParent = pParent->Parent())
		{
			if (predicate(pParent))
			{
				return pParent;
			}
		}
		return NULL;
	}
}