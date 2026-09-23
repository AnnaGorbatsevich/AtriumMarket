import React from 'react';

// Renders inside a flex row: the button, the "already in cart" badge and an error, if any.
const AddToCartControl = ({ variant, status, inCart, onAdd }) => {
  const isAdding = status === 'adding';
  const isError = status && !isAdding;
  const isSoldOut = variant.quantity === 0;
  const isLimitReached = !isSoldOut && inCart >= variant.quantity;

  return (
    <>
      <button
        type="button"
        className="add-variant-btn"
        style={{ width: 'auto', margin: 0, padding: '0.25rem 0.6rem', fontSize: '0.75rem' }}
        disabled={isAdding || isSoldOut || isLimitReached}
        onClick={(e) => {
          e.stopPropagation();
          onAdd();
        }}
      >
        {isAdding ? 'Добавление...' : 'В корзину'}
      </button>
      {inCart > 0 && <span className="price-badge">Уже в корзине: {inCart} шт.</span>}
      {isError && <span className="field-error">{status}</span>}
    </>
  );
};

export default AddToCartControl;
