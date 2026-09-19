import React, { useCallback, useEffect, useState } from 'react';
import { getJson, authGet, authPost } from '../api';
import { getToken } from '../auth';
import { pluralizeVariants, formatOptions, minPrice } from '../productDisplay';

const addToCart = (payload) => authPost('/add_basket', getToken(), payload);

// Total quantity per variant (older data may hold several cart rows for one variant).
const countByVariant = (basket) => {
  const counts = {};
  basket.forEach((item) => {
    counts[item.variantId] = (counts[item.variantId] || 0) + item.quantity;
  });
  return counts;
};

const CatalogPage = () => {
  const [categories, setCategories] = useState([]);
  const [categoryId, setCategoryId] = useState('');
  const [products, setProducts] = useState([]);
  const [loading, setLoading] = useState(true);
  const [loadError, setLoadError] = useState('');
  const [cartStatus, setCartStatus] = useState({});
  const [cartCounts, setCartCounts] = useState({});

  useEffect(() => {
    getJson('/categories')
      .then(setCategories)
      .catch(() => setCategories([]));
  }, []);

  useEffect(() => {
    authGet('/get_basket', getToken())
      .then((basket) => setCartCounts(countByVariant(basket)))
      .catch(() => setCartCounts({}));
  }, []);

  const loadProducts = useCallback(() => {
    setLoading(true);
    setLoadError('');
    const path = categoryId ? `/catalog?categoryId=${categoryId}` : '/catalog';
    getJson(path)
      .then(setProducts)
      .catch((err) => setLoadError(err.message))
      .finally(() => setLoading(false));
  }, [categoryId]);

  useEffect(() => {
    loadProducts();
  }, [loadProducts]);

  const handleAddToCart = async (product, variant) => {
    setCartStatus((prev) => ({ ...prev, [variant.id]: 'adding' }));
    try {
      await addToCart({
        sellerId: product.sellerId,
        variantId: variant.id,
        quantity: 1,
        price: variant.price,
      });
      setCartStatus((prev) => ({ ...prev, [variant.id]: undefined }));
      setCartCounts((prev) => ({ ...prev, [variant.id]: (prev[variant.id] || 0) + 1 }));
    } catch (err) {
      setCartStatus((prev) => ({ ...prev, [variant.id]: err.message }));
    }
  };

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Поиск товаров</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          <div className="form-group">
            <label>Категория</label>
            <select className="form-control" value={categoryId} onChange={(e) => setCategoryId(e.target.value)}>
              <option value="">Все категории</option>
              {categories.map((category) => (
                <option key={category.id} value={category.id}>
                  {category.name}
                </option>
              ))}
            </select>
          </div>

          {loading && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Загрузка...
            </p>
          )}

          {!loading && loadError && <div className="field-error submit-error">{loadError}</div>}

          {!loading && !loadError && products.length === 0 && (
            <p className="success-subtext" style={{ textAlign: 'center' }}>
              Товары не найдены.
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
                    {product.variants.map((variant) => {
                      const status = cartStatus[variant.id];
                      const isAdding = status === 'adding';
                      const isError = status && !isAdding;
                      const inCart = cartCounts[variant.id] || 0;
                      return (
                        <div
                          key={variant.id}
                          style={{
                            display: 'flex',
                            alignItems: 'center',
                            flexWrap: 'wrap',
                            gap: '0.5rem',
                            margin: '0.25rem 0',
                          }}
                        >
                          <div className="success-subtext" style={{ margin: 0, fontSize: '0.8rem' }}>
                            {formatOptions(variant.options)}
                            {variant.price} ₽ · в наличии {variant.quantity}
                          </div>
                          <button
                            type="button"
                            className="add-variant-btn"
                            style={{ width: 'auto', margin: 0, padding: '0.25rem 0.6rem', fontSize: '0.75rem' }}
                            disabled={isAdding || variant.quantity === 0}
                            onClick={() => handleAddToCart(product, variant)}
                          >
                            {isAdding ? 'Добавление...' : 'В корзину'}
                          </button>
                          {inCart > 0 && <span className="price-badge">Уже в корзине: {inCart} шт.</span>}
                          {isError && <span className="field-error">{status}</span>}
                        </div>
                      );
                    })}
                  </div>
                </div>
                <span className="price-badge">от {minPrice(product.variants)} ₽</span>
              </div>
            ))}
        </div>
      </div>
    </div>
  );
};

export default CatalogPage;
