import React from 'react';

export const ROLES = {
  buyer: {
    title: 'Покупатель',
    description: 'Покупайте товары у продавцов на AtriumMarket',
    icon: (
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
        <circle cx="9" cy="21" r="1" />
        <circle cx="20" cy="21" r="1" />
        <path d="M1 1h4l2.68 13.39a2 2 0 0 0 2 1.61h9.72a2 2 0 0 0 2-1.61L23 6H6" />
      </svg>
    ),
  },
  seller: {
    title: 'Продавец',
    description: 'Продавайте свои товары и управляйте магазином',
    icon: (
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
        <path d="M3 9l1.5-5h15L21 9" />
        <path d="M3 9h18v10a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V9z" />
        <path d="M8 13a2 2 0 0 0 4 0" />
      </svg>
    ),
  },
};

const RoleModal = ({ onSelect }) => (
  <div className="modal-overlay">
    <div className="modal-card">
      <div className="header">
        <h1>AtriumMarket</h1>
      </div>
      <div className="card-body" style={{ padding: '2rem' }}>
        <p className="modal-subtitle">Выберите тип аккаунта, чтобы продолжить регистрацию</p>
        <div className="role-grid">
          {Object.entries(ROLES).map(([key, role]) => (
            <button key={key} className="role-card" onClick={() => onSelect(key)}>
              <span className="role-icon">{role.icon}</span>
              <span className="role-title">{role.title}</span>
              <span className="role-description">{role.description}</span>
            </button>
          ))}
        </div>
      </div>
    </div>
  </div>
);

export default RoleModal;
