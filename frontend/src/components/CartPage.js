import React, { useCallback, useEffect, useMemo, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { getToken } from '../auth';
import { authGet, authPost, getJson } from '../api';
import { TrashIcon } from './icons';
import ProductCard from './ProductCard';

// The gateway identifies the caller via the Authorization token (GetMe) - no id is sent here.
const fetchBasket = (token) => authGet('/get_basket', token);
const updateQuantity = (variantId, action) => authPost('/update_basket', getToken(), { variantId, action });
const addToCart = (payload) => authPost('/add_basket', getToken(), payload);
const placeOrder = () => authPost('/checkout', getToken(), {});

const buildVariantIndex = (products) => {
  const index = {};
  products.forEach((product) => {
    product.variants.forEach((variant) => {
      index[variant.id] = { name: product.name, options: variant.options, product };
    });
  });
  return index;
};

const mergeByVariant = (items) => {
  const merged = new Map();
  items.forEach((item) => {
    const existing = merged.get(item.variantId);
    if (existing) {
      existing.quantity += item.quantity;
      existing.price = item.price;
    } else {
      merged.set(item.variantId, { ...item });
    }
  });
  return [...merged.values()];
};

const describeOptions = (options) =>
  options
    ? Object.entries(options)
        .map(([key, value]) => `${key}: ${value}`)
        .join(', ')
    : '';

const formatPrice = (value) => `${value.toLocaleString('ru-RU')} ₽`;

const CartPage = () => {
  const [items, setItems] = useState([]);
  const [variantIndex, setVariantIndex] = useState({});
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');
  const [busyVariantId, setBusyVariantId] = useState(null);
  const [actionError, setActionError] = useState('');
  const [selectedProduct, setSelectedProduct] = useState(null);
  const [cardStatus, setCardStatus] = useState({});
  const [checkingOut, setCheckingOut] = useState(false);
  const navigate = useNavigate();
  const closeProductCard = useCallback(() => setSelectedProduct(null), []);

  const loadBasket = useCallback(() => {
    setLoading(true);
    setLoadError('');
    Promise.all([fetchBasket(getToken()), getJson('/catalog').catch(() => [])])
      .then(([basket, catalog]) => {
        setItems(mergeByVariant(basket));
        setVariantIndex(buildVariantIndex(catalog));
      })
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, []);

  useEffect(() => {
    loadBasket();
  }, [loadBasket]);

  // action is 'increase' or 'decrease' - the server only ever steps the cart by one unit at a
  // time (see order_service::OrderDAO::UpdateCartQuantity), so there's no absolute quantity to send.
  const changeQuantity = async (item, action) => {
    setActionError('');
    setBusyVariantId(item.variantId);
    try {
      await updateQuantity(item.variantId, action);
      setItems((prev) => {
        if (action === 'decrease' && item.quantity <= 1) {
          return prev.filter((entry) => entry.variantId !== item.variantId);
        }
        const delta = action === 'increase' ? 1 : -1;
        return prev.map((entry) =>
          entry.variantId === item.variantId ? { ...entry, quantity: entry.quantity + delta } : entry
        );
      });
    } catch (err) {
      setActionError(err.message);
    } finally {
      setBusyVariantId(null);
    }
  };

  const cartCounts = useMemo(() => {
    const counts = {};
    items.forEach((item) => {
      counts[item.variantId] = item.quantity;
    });
    return counts;
  }, [items]);

  const addFromCard = async (product, variant) => {
    setCardStatus((prev) => ({ ...prev, [variant.id]: 'adding' }));
    try {
      await addToCart({
        sellerId: product.sellerId,
        variantId: variant.id,
        price: variant.price,
      });
      setCardStatus((prev) => ({ ...prev, [variant.id]: undefined }));
      setItems((prev) =>
        prev.some((entry) => entry.variantId === variant.id)
          ? prev.map((entry) =>
              entry.variantId === variant.id
                ? { ...entry, quantity: entry.quantity + 1, price: variant.price }
                : entry
            )
          : [...prev, { variantId: variant.id, sellerId: product.sellerId, quantity: 1, price: variant.price }]
      );
    } catch (err) {
      setCardStatus((prev) => ({ ...prev, [variant.id]: err.message }));
    }
  };

  const checkout = async () => {
    setActionError('');
    setCheckingOut(true);
    try {
      await placeOrder();
      navigate('/orders');
    } catch (err) {
      setActionError(err.message);
      setCheckingOut(false);
    }
  };

  const total = items.reduce((sum, item) => sum + item.quantity * item.price, 0);
  const totalCount = items.reduce((sum, item) => sum + item.quantity, 0);

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

          {!loading && !loadError && actionError && <div className="field-error submit-error">{actionError}</div>}

          {!loading && !loadError && items.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Корзина пуста. Добавьте товары на странице поиска.
            </p>
          )}

          {!loading &&
            !loadError &&
            items.map((item) => {
              const info = variantIndex[item.variantId];
              const options = info ? describeOptions(info.options) : '';
              const busy = busyVariantId === item.variantId;
              return (
                <div
                  className={`product-list-item cart-row${info ? ' clickable' : ''}`}
                  key={item.variantId}
                  onClick={info ? () => setSelectedProduct(info.product) : undefined}
                >
                  <div className="cart-row-info">
                    <div style={{ fontWeight: 600 }}>{info ? info.name : `Товар (вариант #${item.variantId})`}</div>
                    {options && (
                      <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                        {options}
                      </div>
                    )}
                    <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                      {formatPrice(item.price)} за шт.
                    </div>
                  </div>

                  <div className="qty-control" onClick={(e) => e.stopPropagation()}>
                    {item.quantity > 1 ? (
                      <button
                        type="button"
                        className="qty-btn"
                        aria-label="Уменьшить количество"
                        disabled={busy}
                        onClick={() => changeQuantity(item, 'decrease')}
                      >
                        −
                      </button>
                    ) : (
                      <button
                        type="button"
                        className="qty-btn remove"
                        aria-label="Удалить из корзины"
                        title="Удалить из корзины"
                        disabled={busy}
                        onClick={() => changeQuantity(item, 'decrease')}
                      >
                        <TrashIcon />
                      </button>
                    )}
                    <span className="qty-value">{item.quantity}</span>
                    <button
                      type="button"
                      className="qty-btn"
                      aria-label="Увеличить количество"
                      disabled={busy}
                      onClick={() => changeQuantity(item, 'increase')}
                    >
                      +
                    </button>
                  </div>

                  <div className="cart-row-total">{formatPrice(item.quantity * item.price)}</div>
                </div>
              );
            })}

          {!loading && !loadError && items.length > 0 && (
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
              <span>Итого ({totalCount} шт.)</span>
              <span style={{ fontSize: '1.15rem' }}>{formatPrice(total)}</span>
            </div>
          )}

          {!loading && !loadError && items.length > 0 && (
            <button
              type="button"
              className="btn btn-full"
              style={{ marginTop: '1rem' }}
              disabled={checkingOut || busyVariantId !== null}
              onClick={checkout}
            >
              {checkingOut ? 'Оформление...' : 'Оформить заказ'}
            </button>
          )}
        </div>
      </div>

      {selectedProduct && (
        <ProductCard
          product={selectedProduct}
          cartCounts={cartCounts}
          cartStatus={cardStatus}
          onAddToCart={addFromCard}
          onClose={closeProductCard}
        />
      )}
    </div>
  );
};

export default CartPage;
