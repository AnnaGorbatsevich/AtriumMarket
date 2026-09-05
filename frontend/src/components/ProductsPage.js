import React, { useCallback, useEffect, useState } from 'react';
import { useAuth, getToken } from '../auth';
import { authGet } from '../api';
import AddProductForm from './AddProductForm';

const fetchProducts = () => authGet('/products', getToken());

const pluralizeVariants = (count) => {
  const mod10 = count % 10;
  const mod100 = count % 100;
  if (mod10 === 1 && mod100 !== 11) return 'вариация';
  if ([2, 3, 4].includes(mod10) && ![12, 13, 14].includes(mod100)) return 'вариации';
  return 'вариаций';
};

const formatOptions = (options) => {
  if (!options || Object.keys(options).length === 0) return '';
  return `${Object.entries(options)
    .map(([key, value]) => `${key}: ${value}`)
    .join(', ')} — `;
};

const ProductsPage = () => {
  const { user } = useAuth();
  const [products, setProducts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');
  const [formOpen, setFormOpen] = useState(false);

  const loadProducts = useCallback(() => {
    setLoading(true);
    setLoadError('');
    fetchProducts()
      .then(setProducts)
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, []);

  useEffect(() => {
    if (user.role === 'seller') loadProducts();
  }, [user.role, loadProducts]);

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

  const handleAdded = () => {
    setFormOpen(false);
    loadProducts();
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
          {loading && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Загрузка...
            </p>
          )}

          {!loading && loadError && <div className="field-error submit-error">{loadError}</div>}

          {!loading && !loadError && products.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Пока нет товаров. Нажмите «+ Добавить товар», чтобы создать первый.
            </p>
          )}

          {!loading &&
            !loadError &&
            products.map((product) => (
              <div className="product-list-item" key={product.id} style={{ alignItems: 'flex-start' }}>
                <div>
                  <div style={{ fontWeight: 600 }}>{product.name}</div>
                  {product.description && (
                    <div className="success-subtext" style={{ margin: '0.25rem 0 0' }}>
                      {product.description}
                    </div>
                  )}
                  <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                    {product.categoryName ? `${product.categoryName} · ` : ''}
                    {product.variants.length} {pluralizeVariants(product.variants.length)}
                  </div>
                  <div style={{ marginTop: '0.5rem' }}>
                    {product.variants.map((variant) => (
                      <div key={variant.id} className="success-subtext" style={{ margin: 0, fontSize: '0.8rem' }}>
                        {formatOptions(variant.options)}
                        {variant.price} ₽ · остаток {variant.quantity}
                      </div>
                    ))}
                  </div>
                </div>
                <span className={`status-badge ${product.status}`}>
                  {product.status === 'active' ? 'Активен' : 'Скрыт'}
                </span>
              </div>
            ))}
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
