import React from 'react';

const SuccessCard = ({ result, onRestart }) => (
  <div className="container">
    <div className="card">
      <div className="header">
        <h1>Готово!</h1>
      </div>
      <div className="card-body" style={{ padding: '2rem', textAlign: 'center' }}>
        <div className="success-icon">
          <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <path d="M20 6L9 17l-5-5" />
          </svg>
        </div>
        <p className="success-text">
          Аккаунт {result.role === 'seller' ? 'продавца' : 'покупателя'} «{result.name}» успешно зарегистрирован.
        </p>
        <p className="success-subtext">{result.email}</p>
        <button className="btn" onClick={onRestart}>Зарегистрировать ещё один аккаунт</button>
      </div>
    </div>
  </div>
);

export default SuccessCard;
