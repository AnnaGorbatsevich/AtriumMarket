import React, { useCallback, useEffect, useState } from 'react';
import { useAuth, getToken } from '../auth';
import { authGet, authPost, getJson } from '../api';
import {
  STATUS_LABELS,
  SELLER_ACTIONS,
  BUYER_ACTIONS,
  SELLER_WAITING,
  BUYER_WAITING,
} from '../orderStatus';

const REFRESH_INTERVAL_MS = 15000;

const changeOrderStatus = (orderId, status) => authPost('/update_order_status', getToken(), { orderId, status });

// order_service stores only variant ids, so names and options come from the product lists.
// A buyer only sees active products (the catalog); a seller sees all of their own.
const buildVariantIndex = (products) => {
  const index = {};
  products.forEach((product) => {
    product.variants.forEach((variant) => {
      index[variant.id] = { name: product.name, options: variant.options };
    });
  });
  return index;
};

const describeOptions = (options) =>
  options
    ? Object.entries(options)
        .map(([key, value]) => `${key}: ${value}`)
        .join(', ')
    : '';

const formatPrice = (value) => `${value.toLocaleString('ru-RU')} ₽`;

const OrdersPage = () => {
  const { user } = useAuth();
  const isSeller = user.role === 'seller';
  const actions = isSeller ? SELLER_ACTIONS : BUYER_ACTIONS;
  const waiting = isSeller ? SELLER_WAITING : BUYER_WAITING;

  const [orders, setOrders] = useState([]);
  const [variantIndex, setVariantIndex] = useState({});
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');
  const [statusFilter, setStatusFilter] = useState('');
  const [busyOrderId, setBusyOrderId] = useState(null);
  const [actionError, setActionError] = useState('');

  const fetchOrders = useCallback(
    () => authGet('/get_orders', getToken()).then((data) => setOrders(data)),
    []
  );

  useEffect(() => {
    const products = isSeller ? authGet('/products', getToken()) : getJson('/catalog');
    Promise.all([fetchOrders(), products.catch(() => [])])
      .then(([, productList]) => setVariantIndex(buildVariantIndex(productList)))
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, [isSeller, fetchOrders]);

  // Keeps statuses fresh while the page is open: a seller may change them at any moment.
  useEffect(() => {
    const timer = setInterval(() => {
      if (busyOrderId === null) fetchOrders().catch(() => {});
    }, REFRESH_INTERVAL_MS);
    return () => clearInterval(timer);
  }, [fetchOrders, busyOrderId]);

  const changeStatus = async (order, status) => {
    setActionError('');
    setBusyOrderId(order.id);
    try {
      await changeOrderStatus(order.id, status);
      setOrders((prev) => prev.map((entry) => (entry.id === order.id ? { ...entry, status } : entry)));
    } catch (err) {
      setActionError(err.message);
      fetchOrders().catch(() => {});
    } finally {
      setBusyOrderId(null);
    }
  };

  const visibleOrders = statusFilter ? orders.filter((order) => order.status === statusFilter) : orders;

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>{isSeller ? 'Заказы' : 'Мои заказы'}</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {!loading && !loadError && orders.length > 0 && (
            <div className="form-group">
              <label>Статус</label>
              <select className="form-control" value={statusFilter} onChange={(e) => setStatusFilter(e.target.value)}>
                <option value="">Все</option>
                {Object.entries(STATUS_LABELS).map(([status, label]) => (
                  <option key={status} value={status}>
                    {label}
                  </option>
                ))}
              </select>
            </div>
          )}

          {loading && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Загрузка...
            </p>
          )}

          {!loading && loadError && <div className="field-error submit-error">{loadError}</div>}

          {!loading && !loadError && actionError && <div className="field-error submit-error">{actionError}</div>}

          {!loading && !loadError && orders.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              {isSeller
                ? 'Заказов на ваши товары пока нет.'
                : 'Заказов пока нет. Добавьте товары в корзину и оформите заказ.'}
            </p>
          )}

          {!loading && !loadError && orders.length > 0 && visibleOrders.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Нет заказов с таким статусом.
            </p>
          )}

          {!loading &&
            !loadError &&
            visibleOrders.map((order) => {
              const info = variantIndex[order.variantId];
              const options = info ? describeOptions(info.options) : '';
              const orderActions = actions[order.status] || [];
              const busy = busyOrderId === order.id;
              return (
                <div className="product-list-item cart-row" key={order.id}>
                  <div className="cart-row-info">
                    <div style={{ fontWeight: 600 }}>{info ? info.name : `Товар (вариант #${order.variantId})`}</div>
                    {options && (
                      <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                        {options}
                      </div>
                    )}
                    <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                      Заказ №{order.id} · {formatPrice(order.price)} × {order.quantity} шт.
                      {isSeller ? ` · Покупатель №${order.buyerId}` : ''}
                    </div>
                    {orderActions.length === 0 && waiting[order.status] && (
                      <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                        {waiting[order.status]}
                      </div>
                    )}
                  </div>

                  <div className="cart-row-total">{formatPrice(order.quantity * order.price)}</div>

                  <div className="order-actions">
                    <span className={`status-badge ${order.status}`}>{STATUS_LABELS[order.status] || order.status}</span>
                    {orderActions.map((action) => (
                      <button
                        key={action.status}
                        type="button"
                        className={`order-action-btn${action.danger ? ' danger' : ''}`}
                        disabled={busy}
                        onClick={() => changeStatus(order, action.status)}
                      >
                        {action.label}
                      </button>
                    ))}
                  </div>
                </div>
              );
            })}
        </div>
      </div>
    </div>
  );
};

export default OrdersPage;
