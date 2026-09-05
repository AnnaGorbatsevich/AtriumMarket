import React, { useState } from 'react';
import { useAuth } from '../auth';
import AddProductForm from './AddProductForm';

const ProductsPage = () => {
  const { user } = useAuth();
  const [products, setProducts] = useState([]);
  const [formOpen, setFormOpen] = useState(false);

  if (user.role !== 'seller') {
    return (
      <div className="container">
        <div className="card">
          <div className="header">
            <h1>Товары</h1>
          </div>
          <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
            <p className="success-subtext">Этот раздел доступен только продавцам.</p>
          </div>
        </div>
      </div>
    );
  }

  const handleAdded = (product) => {
    setProducts((prev) => [product, ...prev]);
    setFormOpen(false);
  };

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Товары</h1>
          <button type="button" className="change-role-btn" onClick={() => setFormOpen(true)}>
            + Добавить товар
          </button>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {products.length === 0 ? (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Пока нет товаров. Нажмите «+ Добавить товар», чтобы создать первый.
            </p>
          ) : (
            <>
              <p className="success-subtext">Товары, добавленные в этой сессии:</p>
              {products.map((product) => (
                <div className="product-list-item" key={product.productId}>
                  <div>
                    <div>{product.name}</div>
                    <div className="success-subtext" style={{ margin: 0, fontSize: '0.8rem' }}>
                      Вариаций: {product.variantsCount}
                    </div>
                  </div>
                  <span className={`status-badge ${product.status}`}>
                    {product.status === 'active' ? 'Активен' : 'Скрыт'}
                  </span>
                </div>
              ))}
            </>
          )}
        </div>
      </div>

      {formOpen && (
        <div className="modal-overlay" onClick={() => setFormOpen(false)}>
          <div className="modal-card" onClick={(e) => e.stopPropagation()}>
            <div className="header">
              <h1>Новый товар</h1>
            </div>
            <div className="card-body" style={{ padding: '2rem' }}>
              <AddProductForm onAdded={handleAdded} onCancel={() => setFormOpen(false)} />
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default ProductsPage;
