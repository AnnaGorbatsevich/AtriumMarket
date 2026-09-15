import React, { useCallback, useEffect, useState } from 'react';
import { getToken } from '../auth';
import { authGet } from '../api';

// The gateway identifies the caller via the Authorization token (GetMe) - no id is sent here.
const fetchBasket = (token) => authGet('/get_basket', token);

const CartPage = () => {
  const [items, setItems] = useState([]);
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');

  const loadBasket = useCallback(() => {
    setLoading(true);
    setLoadError('');
    fetchBasket(getToken())
      .then(setItems)
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, []);

  useEffect(() => {
    loadBasket();
  }, [loadBasket]);

  const total = items.reduce((sum, item) => sum + item.quantity * item.price, 0);

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Корзина</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {loading && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Загрузка...
            </p>
          )}

          {!loading && loadError && <div className="field-error submit-error">{loadError}</div>}

          {!loading && !loadError && items.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Корзина пуста. Добавьте товары на странице поиска.
            </p>
          )}

          {!loading &&
            !loadError &&
            items.map((item) => (
              <div className="product-list-item" key={item.id}>
                <div>
                  <div style={{ fontWeight: 600 }}>Вариант #{item.variantId}</div>
                  <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                    {item.quantity} × {item.price} ₽
                  </div>
                </div>
                <span className="price-badge">{item.quantity * item.price} ₽</span>
              </div>
            ))}

          {!loading && !loadError && items.length > 0 && (
            <div
              style={{
                display: 'flex',
                justifyContent: 'space-between',
                marginTop: '1rem',
                paddingTop: '1rem',
                borderTop: '1px solid #e2e8f0',
                fontWeight: 600,
              }}
            >
              <span>Итого</span>
              <span>{total} ₽</span>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default CartPage;
