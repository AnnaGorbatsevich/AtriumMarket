import React, { useEffect, useState } from 'react';
import { getToken } from '../auth';
import { authGet } from '../api';
import { STATUS_LABELS } from '../orderStatus';

const formatPrice = (value) => `${value.toLocaleString('ru-RU')} ₽`;

const StatsPage = () => {
  const [stats, setStats] = useState([]);
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');

  useEffect(() => {
    authGet('/seller_stats', getToken())
      .then((data) => setStats(data))
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, []);

  const totalOrders = stats.reduce((sum, entry) => sum + entry.orders, 0);
  const totalRevenue = stats.reduce((sum, entry) => sum + entry.revenue, 0);

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Статистика</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {loading && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Загрузка...
            </p>
          )}

          {!loading && loadError && <div className="field-error submit-error">{loadError}</div>}

          {!loading && !loadError && stats.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Пока нет данных для статистики. Она появится после первых заказов.
            </p>
          )}

          {!loading && !loadError && stats.length > 0 && (
            <>
              {stats.map((entry) => (
                <div className="product-list-item cart-row" key={entry.status}>
                  <div className="cart-row-info">
                    <span className={`status-badge ${entry.status}`}>
                      {STATUS_LABELS[entry.status] || entry.status}
                    </span>
                    <div className="success-subtext" style={{ margin: '0.5rem 0 0', fontSize: '0.8rem' }}>
                      Заказов: {entry.orders}
                    </div>
                  </div>
                  <div className="cart-row-total">{formatPrice(entry.revenue)}</div>
                </div>
              ))}

              <div
                style={{
                  display: 'flex',
                  justifyContent: 'space-between',
                  alignItems: 'baseline',
                  marginTop: '1rem',
                  paddingTop: '1rem',
                  borderTop: '1px solid #e2e8f0',
                  fontWeight: 600,
                }}
              >
                <span>Итого ({totalOrders} заказ.)</span>
                <span style={{ fontSize: '1.15rem' }}>{formatPrice(totalRevenue)}</span>
              </div>
            </>
          )}
        </div>
      </div>
    </div>
  );
};

export default StatsPage;
