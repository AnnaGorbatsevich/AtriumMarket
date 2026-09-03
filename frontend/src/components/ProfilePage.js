import React from 'react';
import { useAuth } from '../auth';

const ProfilePage = () => {
  const { user } = useAuth();

  return (
    <div className="container">
      <div className="card">
        <div className="header">
          <h1>Профиль</h1>
        </div>
        <div className="card-body" style={{ padding: '2rem' }}>
          <p>
            <strong>Имя:</strong> {user.fullName}
          </p>
          <p>
            <strong>Email:</strong> {user.email}
          </p>
          <p>
            <strong>Роль:</strong> {user.role === 'seller' ? 'Продавец' : 'Покупатель'}
          </p>
        </div>
      </div>
    </div>
  );
};

export default ProfilePage;
