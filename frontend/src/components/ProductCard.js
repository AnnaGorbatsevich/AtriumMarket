import React, { useEffect } from 'react';
import AddToCartControl from './AddToCartControl';

const describeOptions = (options) =>
  options
    ? Object.entries(options)
        .map(([key, value]) => `${key}: ${value}`)
        .join(', ')
    : '';

const ProductCard = ({ product, cartCounts, cartStatus, onAddToCart, onClose }) => {
  useEffect(() => {
    const onKeyDown = (e) => {
      if (e.key === 'Escape') onClose();
    };
    window.addEventListener('keydown', onKeyDown);
    return () => window.removeEventListener('keydown', onKeyDown);
  }, [onClose]);

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-card" onClick={(e) => e.stopPropagation()}>
        <div className="header">
          <h1>{product.name}</h1>
          <button type="button" className="change-role-btn" onClick={onClose}>
            Закрыть
          </button>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          {product.categoryName && (
            <span className="price-badge" style={{ marginBottom: '0.75rem' }}>
              {product.categoryName}
            </span>
          )}
          <p className="success-subtext" style={{ margin: '0 0 1.5rem' }}>
            {product.description || 'Описание не указано.'}
          </p>

          <div style={{ fontWeight: 600, marginBottom: '0.5rem' }}>Варианты</div>
          {product.variants.map((variant) => (
            <div className="product-list-item cart-row" key={variant.id}>
              <div className="cart-row-info">
                <div style={{ fontWeight: 600 }}>{describeOptions(variant.options) || 'Стандартный вариант'}</div>
                <div className="success-subtext" style={{ margin: '0.25rem 0 0', fontSize: '0.8rem' }}>
                  в наличии {variant.quantity}
                </div>
              </div>
              <div style={{ fontWeight: 700, fontSize: '1.05rem' }}>{variant.price} ₽</div>
              <div style={{ display: 'flex', alignItems: 'center', flexWrap: 'wrap', gap: '0.5rem' }}>
                <AddToCartControl
                  variant={variant}
                  status={cartStatus[variant.id]}
                  inCart={cartCounts[variant.id] || 0}
                  onAdd={() => onAddToCart(product, variant)}
                />
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default ProductCard;
