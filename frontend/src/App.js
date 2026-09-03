import React from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import './App.css';
import RegisterPage from './components/RegisterPage';
import LoginPage from './components/LoginPage';
import Layout from './components/Layout';
import ProfilePage from './components/ProfilePage';
import Placeholder from './components/Placeholder';
import { AuthProvider, useAuth } from './auth';

const RequireAuth = ({ children }) => {
  const { status, user } = useAuth();
  if (status === 'loading') return null;
  return user ? children : <Navigate to="/login" replace />;
};

const RequireGuest = ({ children }) => {
  const { status, user } = useAuth();
  if (status === 'loading') return null;
  return user ? <Navigate to="/" replace /> : children;
};

const App = () => (
  <AuthProvider>
    <BrowserRouter>
      <Routes>
        <Route
          path="/"
          element={
            <RequireAuth>
              <Layout />
            </RequireAuth>
          }
        >
          <Route index element={<Navigate to="/profile" replace />} />
          <Route path="profile" element={<ProfilePage />} />
          <Route path="cart" element={<Placeholder title="Корзина" />} />
          <Route path="search" element={<Placeholder title="Поиск Товаров" />} />
          <Route path="statistics" element={<Placeholder title="Статистика" />} />
          <Route path="favorites" element={<Placeholder title="Избранное" />} />
          <Route path="orders" element={<Placeholder title="Заказы" />} />
          <Route path="products" element={<Placeholder title="Товары" />} />
          <Route path="messages" element={<Placeholder title="Сообщения" />} />
          <Route path="notifications" element={<Placeholder title="Уведомления" />} />
        </Route>
        <Route path="/login" element={<RequireGuest><LoginPage /></RequireGuest>} />
        <Route path="/register" element={<RequireGuest><RegisterPage /></RequireGuest>} />
        <Route path="*" element={<Navigate to="/" replace />} />
      </Routes>
    </BrowserRouter>
  </AuthProvider>
);

export default App;
